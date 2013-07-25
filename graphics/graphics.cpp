#include "graphics.h"

#include "translate/objectcode.h"
#include "translate/shadersource.h"
#include "translate/mxassembly.h"
#include "model_mx.h"

#include "gui/maingui.h"
#include "translate/guiassembly.h"

#include <iostream>
#include <memory>

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
    gui( vsHolder, fsHolder, vboHolder, iboHolder, wndSize, lang() ),
    mat( vsHolder, fsHolder ){
  time = 0;

  ObjectCode co;
  co.wrkFolder = std::make_shared<std::wstring>(L"./data/sh/material");
  {
  co.loadFromFile( L"data/sh/material/bump.json" );

  MxAssembly assemb( MVertex::decl(), lang() );
  mat.install( *co.codeOf(), assemb );
  }
  }

bool Graphics::render( Scene &scene,
                       ParticleSystemEngine &e,
                       const Tempest::Camera& camera,
                       size_t dt) {
  if( !device.startRender() )
    return false;

  onRender( std::max<size_t>(dt-time, 0) );
  gui.update( *widget, device );

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

  /*
  gui.reset( new GUIPass( res.vshader("gui"),
                          res.fshader("gui"),
                          lvboHolder,
                          liboHolder,
                          windSize ) );*/
  }

void Graphics::renderSubScene( const Scene &/*scene*/,
                               ParticleSystemEngine &,
                               Tempest::Texture2d &/*out*/ ) {

  }

void Graphics::setFog(const Tempest::Pixmap &) {
  }

void Graphics::setSettings(const GraphicsSettingsWidget::Settings &) {
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

  Frustum frustum( camera );
  draw( frustum, true, camera, scene.all() );
  completeDraw(scene);

  gui.exec( *widget, 0, 0, device );
  }

void Graphics::setupSceneConstants( Scene& scene,
                                    ShaderMaterial::UniformsContext& context ) {
  Tempest::Matrix4x4& mWorld = context.mWorld;
  mWorld = scene.camera().view();

  const Tempest::Matrix4x4& vm = mWorld;

  float view[3] = { float(vm.at(2,0)),
                    float(vm.at(2,1)),
                    float(vm.at(2,2)) };
  float len = sqrt(view[0]*view[0] + view[1]*view[1] + view[2]*view[2]);

  for( int i=0; i<3; ++i )
    view[i] /= -len;
  std::copy( view, view+3, context.view );
  }

void Graphics::setupObjectConstants( Scene& scene,
                                     const AbstractGraphicObject &obj,
                                     ShaderMaterial::UniformsContext& context ){
  Tempest::Matrix4x4& mWorld = context.mWorld,
                    & object = context.object;

  object = obj.transform();

  Tempest::Matrix4x4& mvp = context.mvp;
  mvp = scene.camera().projective();
  mvp.mul( mWorld );
  mvp.mul( object );

  context.texture[0] = obj.material().diffuse;
  context.texture[1] = obj.material().normal;
  context.texID = 0;
  }

void Graphics::completeDraw( Scene & scene ) {
  //std::sort( toDraw.begin(), toDraw.end(), cmp );

  //Tempest::UniformTable table( render );
  //Tempest::RenderState rs;

  setupSceneConstants( scene, context );

  Tempest::Render render(device, mat.vs, mat.fs);

  for( size_t i=0; i<toDraw.size(); ++i ){
    const AbstractGraphicObject& ptr = *toDraw[i];
    //render.setRenderState(rs);
    setupObjectConstants(scene, ptr, context);
    mat.setupShaderConst(context);
    render.draw( ptr );
    }

  toDraw.clear();
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

int Graphics::draw( const Frustum &frustum,
                    bool deepVTest,
                    const Tempest::AbstractCamera & camera,
                    const Scene::Objects & v ){
  int c = 0;
  for( size_t i=0; i<v.objects.size(); ++i ){
    const AbstractGraphicObject& ptr = *v.objects[i];

    if( !deepVTest || isVisible(ptr,frustum) ){
      toDraw.push_back( v.objects[i] );
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
              c+=draw( frustum, ret!=FullVisible, camera, t );
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
  if( GraphicsSettingsWidget::Settings::api ==
      GraphicsSettingsWidget::Settings::directX )
    return ShaderSource::Cg; else
    return ShaderSource::GLSL;
  }

Tempest::Device::Options Graphics::makeOpt(bool isFullScreen) {
  Tempest::Device::Options opt;
  opt.windowed = !isFullScreen;
  opt.vSync    = !opt.windowed;

  return opt;
  }
