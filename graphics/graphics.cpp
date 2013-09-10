#include "graphics.h"

#include "translate/objectcode.h"
#include "translate/shadersource.h"
#include "translate/mxassembly.h"
#include "model_mx.h"

#include "gui/maingui.h"
#include "translate/guiassembly.h"
#include "gbufffer.h"

#include <iostream>
#include <memory>
#include <cmath>

#include <Tempest/DisplaySettings>

Graphics::Graphics( void *hwnd, bool isFullScreen )
  : api( createAPI() ),
    device( *api, makeOpt(isFullScreen), hwnd ),
    texHolder ( device ),
    localTex  ( device ),
    vboHolder ( device ),
    lvboHolder( device ),
    iboHolder ( device ),
    liboHolder( device ),
    vsHolder  ( device ),
    fsHolder  ( device ),
    //gui( vsHolder, fsHolder, vboHolder, iboHolder, wndSize, lang() ),
    r0( vsHolder, fsHolder ),
    uiRender( vsHolder, fsHolder ),
    hintRender( vsHolder, fsHolder ),
    msrv( vsHolder, fsHolder, localTex, lang() ){
  time   = 0;
  useHDR = 0;

#ifdef __ANDROID__
  lvboHolder.setReserveSize( 8092 );
  lvboHolder.setMaxReservedCount( 1 );
  liboHolder.setReserveSize( 8092 );
  liboHolder.setMaxReservedCount( 1 );
  localTex.setMaxCollectIterations(-1);
#else
  lvboHolder.setReserveSize( 64*8092 );
  lvboHolder.setMaxReservedCount( -1 );
  liboHolder.setReserveSize( 64*8092 );
  liboHolder.setMaxReservedCount( -1 );
#endif
  //Tempest::DisplaySettings s( Tempest::SystemAPI::screenSize(), 32, true );
  //Tempest::DisplaySettings s( 1280, 768, 32, true );

  //device.setDisplaySettings(s);
  }

bool Graphics::render( Tempest::Surface &scene,
                       Tempest::SpritesHolder & sp ) {
  if( !device.startRender() )
    return false;

  device.clear( Tempest::Color(1,0,0), 1 );
  uiRender.buildVbo(scene, vboHolder, iboHolder, sp );

  device.beginPaint();
  uiRender.renderTo(device);
  device.endPaint();

  device.present();
  return true;
  }

bool Graphics::render( Scene &scene,
                       ParticleSystemEngine &e,
                       const Tempest::Camera& camera,
                       size_t dt) {
  if( !device.startRender() )
    return false;

  onRender( std::max<size_t>(dt-time, 0) );
  //gui.update( *widget, device );
  widget->buildVBO( r0,
                    uiRender,
                    hintRender,
                    lvboHolder,
                    liboHolder);

  time = dt;//(time+dt);
  renderImpl(scene, e, camera, dt);
  device.present();

  return true;
  }

void Graphics::resizeEvent(int w, int h, bool isFullScreen) {
  wndSize = Tempest::Size(w,h);
  device.reset( makeOpt(isFullScreen) );
  }

void Graphics::load( Resource &res, MainGui &ui, int w, int h) {
  resource = &res;
  widget   = &ui;
  wndSize = Tempest::Size(w,h);

  msrv.load(res);
  }

void Graphics::renderSubScene( const Scene &scene,
                               ParticleSystemEngine &,
                               Tempest::Texture2d &out ) {
  Tempest::Texture2d d = depth(out.size());

  device.beginPaint(out, d);
  device.clear( Tempest::Color(0), 1 );
  device.endPaint();

  context.invW = 1.0f/out.width();
  context.invH = 1.0f/out.height();

  cefects.invW = context.invW;
  cefects.invH = context.invH;

  context.texture[ ShaderSource::tsShadowMap ][0]
      = fillShadowMap(device, scene, Tempest::Size(128));

  Frustum frustum( scene.camera() );
  draw( frustum, true, false, scene.all() );

  GBuffer g;
  initGB(g, out.size());

  setupSceneConstants( scene, context );
  msrv.completeDraw(device, scene, g, &out, &d, context, cefects);

  for( int i=0; i<ShaderSource::tsCount; ++i )
    for( int r=0; r<32; ++r ){
      context.texture[i][r] = Tempest::Texture2d();
      cefects.texture[i][r] = Tempest::Texture2d();
      }
  }

void Graphics::setFog(const Tempest::Pixmap &) {
  }

void Graphics::setSettings(const GraphicsSettingsWidget::Settings &s) {
  settings = s;
  msrv.setSettings(s);
  }

void Graphics::renderImpl( Scene &scene,
                           ParticleSystemEngine &,
                           const Tempest::Camera& camera,
                           size_t /*dt*/) {
  device.beginPaint();
  device.clear( Tempest::Color(0,0,1), 1 );
  device.endPaint();

  context.invW = 1.0f/wndSize.w;
  context.invH = 1.0f/wndSize.h;

  cefects.invW = context.invW;
  cefects.invH = context.invH;

  context.texture[ ShaderSource::tsShadowMap ][0]
      = fillShadowMap(device, scene, Tempest::Size(settings.shadowMapRes));

  Frustum frustum( camera );
  draw( frustum, true, false, scene.all() );

  GBuffer g;
  initGB(g, wndSize);

  setupSceneConstants( scene, context );
  msrv.completeDraw(device, scene, g, 0, 0, context, cefects);

  //gui.exec( *widget, 0, 0, device );
  //surfRender.renderTo( device );
  device.beginPaint();
  r0.renderTo(device);
  uiRender.renderTo(device);
  hintRender.renderTo(device);
  device.endPaint();

  for( int i=0; i<ShaderSource::tsCount; ++i )
    for( int r=0; r<32; ++r ){
      context.texture[i][r] = Tempest::Texture2d();
      cefects.texture[i][r] = Tempest::Texture2d();
      }
  }

Tempest::Texture2d Graphics::fillShadowMap( Tempest::Device & device,
                                            const Scene & scene,
                                            const Tempest::Size& sm ) {
  if( sm.isEmpty() )
    return Tempest::Texture2d();

  Tempest::RenderState rstate;
  rstate.setCullFaceMode( Tempest::RenderState::CullMode::front );

  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  float dir[3] = { float(light.xDirection()),
                   float(light.yDirection()),
                   float(light.zDirection()) };
  Tempest::Matrix4x4 matrix = makeShadowMatrix(scene, dir, 0);

  Tempest::Matrix4x4 proj;
  proj.identity();

  // particles->exec( matrix, proj, 0, true );

  Frustum frustum( matrix );
  draw( frustum, true, true, scene.all() );

  ShadowBuffer g;
  initGB(g, sm );

  context.shView = matrix;
  setupSceneConstants( scene, context, matrix, proj );
  msrv.completeDraw(device, scene, g, context, cefects);

  Tempest::Texture2d::Sampler s = g.z.sampler();
  s.uClamp = Tempest::Texture2d::ClampMode::ClampToBorder;
  s.vClamp = s.uClamp;
  g.z.setSampler(s);

  return g.z;
  }

void Graphics::setupSceneConstants( const Scene& scene,
                                    ShaderMaterial::UniformsContext& context ){
  setupSceneConstants( scene,
                       context,
                       scene.camera().view(),
                       scene.camera().projective() );
  }

void Graphics::setupSceneConstants( const Scene& scene,
                                    ShaderMaterial::UniformsContext& context,
                                    const Tempest::Matrix4x4& world,
                                    const Tempest::Matrix4x4& proj ) {
  Tempest::Matrix4x4& mWorld = context.mWorld;
  mWorld = world;
  context.proj = proj;

  const Tempest::Matrix4x4& vm = mWorld;

  float view[3] = { float(vm.at(2,0)),
                    float(vm.at(2,1)),
                    float(vm.at(2,2)) };
  float len = sqrt(view[0]*view[0] + view[1]*view[1] + view[2]*view[2]);

  for( int i=0; i<3; ++i )
    view[i] /= -len;
  std::copy( view, view+3, context.view );

  uint64_t c = clock();
  c = 1000*c/CLOCKS_PER_SEC;
  context.tick  = c%(16*1024);

  if( scene.lights().direction().size()>0 ){
    Tempest::DirectionLight l = scene.lights().direction()[0];

    double ldir[3] = { l.xDirection(), l.yDirection(), l.zDirection() };
    float  lcl[4]  = { l.color().r(), l.color().g(),
                       l.color().b(), l.color().a() };
    float  lab[4]  = { l.ablimient().r(), l.ablimient().g(),
                       l.ablimient().b(), l.ablimient().a() };

    std::copy( ldir, ldir+3, context.lightDir       );
    std::copy(  lcl,  lcl+3, context.lightColor     );
    std::copy(  lab,  lab+3, context.sceneAblimient );
    }
  }

Graphics::VisibleRet Graphics::isVisible( const AbstractGraphicObject &c,
                                          const Frustum &frustum ) {
  if( !c.isVisible() )
    return NotVisible;

  float r = c.radius(),
        x = c.x() + c.bounds().mid[0]*c.sizeX(),
        y = c.y() + c.bounds().mid[1]*c.sizeY(),
        z = c.z() + c.bounds().mid[2]*c.sizeZ();

  return isVisible(x,y,z, r, frustum);
  }

Graphics::VisibleRet Graphics::isVisible( float x,
                                          float y,
                                          float z,
                                          float r,
                                          const Frustum &frustum ) {
  bool fv = true;
  for( int p=0; p < 6; p++ ){
    float l = frustum.f[p][0] * x +
              frustum.f[p][1] * y +
              frustum.f[p][2] * z +
              frustum.f[p][3];
    if( l <= -r )
      return NotVisible;

    if( !(l >= r) )
      fv = false;
    }

  if( fv )
    return FullVisible; else
    return PartialVisible;
  }

size_t Graphics::idOfMaterial(const std::string &m) {
  return msrv.idOfMaterial(m);
  }

int Graphics::draw( const Frustum &frustum,
                    bool deepVTest,
                    bool shadowPass,
                    const Scene::Objects & v ){
  int c = 0;
  for( size_t i=0; i<v.objects.size(); ++i ){
    const AbstractGraphicObject& ptr = *v.objects[i];

    if( !deepVTest || isVisible(ptr,frustum) ){
      //toDraw.push_back( v.objects[i] );
      msrv.draw( *v.objects[i], shadowPass );
      ++c;
      }
    }

  for( int x=0; x<2; ++x )
    for( int y=0; y<2; ++y )
      for( int z=0; z<2; ++z )
        if( v.nested[x][y][z] ){
          const Scene::Objects &t = *v.nested[x][y][z];

          if( t.count ){
            float dpos = t.linearSize*0.5;

            Graphics::VisibleRet ret = FullVisible;
            if( deepVTest )
              ret = isVisible( t.x+dpos, t.y+dpos, t.z+dpos, t.r, frustum );

            if( ret ){
              c+=draw( frustum, ret!=FullVisible, shadowPass, t );
              }
            }
          }

  return c;
  }

Tempest::AbstractAPI *Graphics::createAPI() {
#ifndef __ANDROID__
  if( GraphicsSettingsWidget::Settings::api ==
      GraphicsSettingsWidget::Settings::directX )
    return new Tempest::DirectX9();
#endif

  return new Tempest::Opengl2x();
  }

ShaderSource::Lang Graphics::lang() const {
#ifdef __ANDROID__
  return CompileOptions::GLSLES;
#endif
  if( GraphicsSettingsWidget::Settings::api ==
      GraphicsSettingsWidget::Settings::directX )
    return CompileOptions::Cg; else
    return CompileOptions::GLSL;
  }

Tempest::Device::Options Graphics::makeOpt(bool isFullScreen) {
  Tempest::Device::Options opt;
  opt.windowed = !isFullScreen;
  opt.vSync    = !opt.windowed;

  return opt;
  }

void Graphics::initGB( GBuffer &b,
                       const Tempest::Size & screenSize ) {
  b.depth = depth( screenSize );

  b.color[0] = colorBuf( screenSize.w, screenSize.h );

#ifndef __ANDROID__
  for( int i=1; i<3; ++i ){
    b.color[i] = colorBuf( screenSize.w, screenSize.h );
    }

  if( GraphicsSettingsWidget::Settings::api
      == GraphicsSettingsWidget::Settings::openGL){
    b.color[3] = colorBuf( screenSize.w, screenSize.h );
    } else {
    if( useHDR )
      b.color[3] = localTex.create( screenSize.w, screenSize.h,
                                    Tempest::Texture2d::Format::RGBA16 ); else
      b.color[3] = localTex.create( screenSize.w, screenSize.h,
                                    Tempest::Texture2d::Format::RG16 );
    }
#endif
  }

void Graphics::initGB(ShadowBuffer &b, const Tempest::Size &size) {
  b.z     = shadowMap(size.w, size.h);
  b.depth = depth(size);
  }

Tempest::Texture2d Graphics::colorBuf(int w, int h) {
#ifdef __ANDROID__
  return localTex.create( w,h, Tempest::Texture2d::Format::RGB5 );
#endif
  if( useHDR )
    return localTex.create( w,h, Tempest::Texture2d::Format::RGBA16 ); else
    return localTex.create( w,h, Tempest::Texture2d::Format::RGBA8 );
  }

Tempest::Texture2d Graphics::depth( const Tempest::Size & sz ) {
  return localTex.create( sz.w, sz.h, Tempest::AbstractTexture::Format::Depth24 );
  }

Tempest::Texture2d Graphics::depth(int w, int h) {
  return localTex.create(w,h, Tempest::AbstractTexture::Format::Depth24 );
  }


Tempest::Texture2d Graphics::shadowMap(int w, int h) {
  if( GraphicsSettingsWidget::Settings::api
      == GraphicsSettingsWidget::Settings::openGL)
    return localTex.create(w,h, Tempest::AbstractTexture::Format::RGBA8 );

  return localTex.create(w,h, Tempest::AbstractTexture::Format::Luminance16 );
  }

Tempest::Matrix4x4 Graphics::makeShadowMatrix( const Scene & scene,
                                               float * dir,
                                               float sv,
                                               bool aspect ) {
  Tempest::Matrix4x4 mat;

  float dist = 0.4, x = 2, y = 2, z = 0, s = 0.3, cs = 0.3;

  const Tempest::Camera &view =
      reinterpret_cast<const Tempest::Camera&>( scene.camera() );

  x = view.x();
  y = view.y();
  z = view.z();

  s = smMatSize(scene, 0, 0);

  float l = sqrt( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );

  for( int i=0; i<3; ++i )
    dir[i] = (dir[i]/l);
  //dir[1] = -dir[1];

  float m[4*4] = {
     s, 0,  0, 0,
     0, s,  0, 0,
     s*dir[0], s*dir[1], cs*dir[2], 0,
     -s*x+s*z, -s*y, s*z+dist, 1,
     };

  mat.setData( m );

  Tempest::Matrix4x4 proj;
  //proj.scale( 1, -1, 1 );

  if( aspect ){
    float a = wndSize.h/float(wndSize.w);
    proj.scale(a,1,1);
    }

#ifdef __ANDROID__
  proj.scale(1.3, 1.3, 1);
#else
  proj.scale(1.15, 1.15, 1);
#endif

  proj.rotateOZ( view.spinX() );
  proj.translate(0,0.1,0);

  if( sv ){
    proj.scale(sv,sv,1);
    }

  proj.mul( mat );
  //proj.set( 3, 1, proj.at(3,1)+0.1 );
  return proj;
  }

float Graphics::smMatSize( const Scene &scene, float sv, float maxSv ) {
  float s = 0.35;
  if( sv>0 )
    s = sv;

  const Tempest::Camera &view =
      reinterpret_cast<const Tempest::Camera&>( scene.camera() );

  s /= std::max( view.distance(), 1.0 )/3.0;
  if( maxSv>0 ){
    s = std::min(s, maxSv);
    } else {
    s = std::min(s, 0.3f);
    }

  return s;
  }

