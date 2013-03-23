#include "graphicssystem.h"

#include <Tempest/Model>
#include <Tempest/Texture2d>
#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

#include <iostream>
#include <cmath>

#include <Tempest/GraphicObject>

#include "graphics/particlesystemengine.h"

#include <resource.h>

GraphicsSystem::GraphicsSystem( void *hwnd,
                                bool isFullScreen,
                                int smSize = 1024 )
  : device( api, makeOpt(isFullScreen), hwnd ),
    texHolder ( device ),
    localTex  ( device ),
    vboHolder ( device ),
    lvboHolder( device ),
    iboHolder ( device ),
    vsHolder  ( device ),
    fsHolder  ( device ),

    ppHelper( vboHolder, iboHolder ){
  widget    = 0;
  time      = -1;
  particles = 0;

  useFog    = false;
  useHDR    = false;

  lvboHolder.setReserveSize( 64*8092 );
  }

void GraphicsSystem::makeRenderAlgo( Resource &res,
                                     MainGui &,
                                     int w, int h ) {
  gui.reset( new GUIPass( res.vshader("gui"),
                          res.fshader("gui"),
                          lvboHolder,
                          screenSize  ) );
  screenSize.w = w;
  screenSize.h = h;

  glowData.vs = res.vshader("glow");
  glowData.fs = res.fshader("glow");

  gbuf.vs     = res.vshader("unit_main_material");
  gbuf.fs     = res.fshader("unit_main_material");

  gbuf.grassVs   = res.vshader("grass_material");
  gbuf.grassFs   = res.fshader("grass_material");

  gbuf.terrainVs = res.vshader("terrain_minor_main_material");
  gbuf.terrainFs = res.fshader("terrain_minor_main_material");

  gbuf.lightDirection.setName("lightDirection");
  gbuf.lightColor    .setName("lightColor");
  gbuf.lightAblimient.setName("lightAblimient");
  gbuf.view          .setName("view");

  smap.vs = vsHolder.load("./data/sh/shadow_map.vert");
  smap.fs = fsHolder.load("./data/sh/shadow_map.frag");

  transparentData.vs = res.vshader("transparent_material");
  transparentData.fs = res.fshader("transparent_material");

  transparentData.vsSh = res.vshader("transparent_material_shadow");
  transparentData.fsSh = res.fshader("transparent_material_shadow");

  transparentData.vsAdd = res.vshader("blend_add");
  transparentData.fsAdd = res.fshader("blend_add");

  displaceData.vs = res.vshader("displace");
  displaceData.fs = res.fshader("displace");

  displaceData.vsWater = res.vshader("water");
  displaceData.fsWater = res.fshader("water");

  gaussData.texture.setName("texture");
  gaussData.vs = res.vshader("gauss");
  gaussData.fs = res.fshader("gauss");

  gaussData.vsGB = res.vshader("gauss_gb");
  gaussData.fsGB = res.fshader("gauss_gb");

  gaussData.vsB = res.vshader("gauss_b");
  gaussData.fsB = res.fshader("gauss_b");

  fogOfWar.vs = res.vshader("fog_of_war");
  fogOfWar.fs = res.fshader("fog_of_war");

  fogOfWar.vsAcept = gaussData.vs;
  fogOfWar.fsAcept = res.fshader("aceptFog");

  volumetricData.vs = gaussData.vs;
  volumetricData.fs = res.fshader("volumetricLight");

  bloomData.vs         = res.vshader("brightPass");
  bloomData.brightPass = res.fshader("brightPass");

  bloomData.combine    = res.fshader("bloomCombine");
  bloomData.b[0].setName("b0");
  bloomData.b[1].setName("b1");
  bloomData.b[2].setName("b2");

  water.waterHeightMap[0] = texHolder.load("./data/textures/water/hmap.png");
  water.waterHeightMap[1] = texHolder.load("./data/textures/water/hmap1.png");

  water.envMap = res.texture("sky0/diff");

  water.vs = vsHolder.load("./data/sh/htonorm.vert");
  water.fs = fsHolder.load("./data/sh/htonorm.frag");

  finalData.vs     = res.vshader("final");
  finalData.fs     = res.fshader("final");
  finalData.avatar = fsHolder.load("./data/sh/avatar_final.frag");

  finalData.scene.setName("scene");
  finalData.bloom.setName("bloom");
  finalData.glow .setName("glow");

  omniData.vs = res.vshader("omni");
  omniData.fs = res.fshader("omni");

  scrOffset.setName("dTexCoord");

  cpyOffset = scrOffset;

  bltData.texture.setName("texture");
  bltData.vs = res.vshader("blt");
  bltData.fs = res.fshader("blt");

  ssaoData.vs       = gaussData.vs;
  ssaoData.fs       = fsHolder.load("./data/sh/ssao_macro.frag");
  ssaoData.detail   = fsHolder.load("./data/sh/ssao_detail.frag");
  ssaoData.accept   = fsHolder.load("./data/sh/ssao_accept.frag");
  ssaoData.acceptGI = fsHolder.load("./data/sh/gi_accept.frag");

  ssaoData.texture.setName("texture");
  ssaoData.blured. setName("blured");
  ssaoData.macro.  setName("macro");
  ssaoData.scene  .setName("scene");
  ssaoData.diff   .setName("diff");
  ssaoData.ssao.   setName("ssao");
  ssaoData.lightAblimient.setName("lightAblimient");

  reflect.uClamp = Tempest::Texture2d::ClampMode::MirroredRepeat;
  reflect.vClamp = reflect.uClamp;

  bufSampler.uClamp = Tempest::Texture2d::ClampMode::ClampToBorder;
  bufSampler.vClamp = bufSampler.uClamp;
  }

float GraphicsSystem::smMatSize( const Scene &scene ) {
    float s = 0.3;

    const Tempest::Camera &view =
        reinterpret_cast<const Tempest::Camera&>( scene.camera() );

    s /= std::max( view.distance(), 1.0 )/3.0;
    s = std::min(s, 0.4f);

    return s;
    }

Tempest::Matrix4x4 GraphicsSystem::makeShadowMatrix( const Scene & scene ){
  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };

  return makeShadowMatrix(scene,dir);
  }

Tempest::Matrix4x4 GraphicsSystem::makeShadowMatrix( const Scene & scene,
                                                  double * dir ){
    Tempest::Matrix4x4 mat;

    float dist = 0.4, x = 2, y = 2, z = 0, s = 0.3, cs = 0.3;

    const Tempest::Camera &view =
        reinterpret_cast<const Tempest::Camera&>( scene.camera() );

    x = view.x();
    y = view.y();
    z = view.z();

    s = smMatSize(scene);

    double l = sqrt( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );

    for( int i=0; i<3; ++i )
      dir[i] = (dir[i]/l);

    double m[4*4] = {
       s, 0,  0, 0,
       0, s,  0, 0,
       s*dir[0], s*dir[1], cs*dir[2], 0,
       -s*x, -s*y, s*z+dist, 1,
       };

    mat.setData( m );

    return mat;
    }

void GraphicsSystem::resizeEvent( int w, int h, bool isFullScreen ){
  screenSize.w = w;
  screenSize.h = h;

  device.reset( makeOpt(isFullScreen) );
  }

Tempest::Device::Options GraphicsSystem::makeOpt(bool isFullScreen) {
  Tempest::Device::Options opt;
  opt.windowed = !isFullScreen;
  opt.vSync    = !opt.windowed;

  return opt;
}

void GraphicsSystem::load( Resource &res, MainGui &gui, int w, int h ) {
  widget = &gui;
  //waterHeightMap = &res.texture("water/height");
  makeRenderAlgo( res, gui, w, h );
  }

Tempest::Texture2d GraphicsSystem::depth(int w, int h) {
  return localTex.create(w,h, Tempest::AbstractTexture::Format::Depth24 );
  }

Tempest::Texture2d GraphicsSystem::depth( const Tempest::Size & sz ) {
  return localTex.create( sz.w, sz.h, Tempest::AbstractTexture::Format::Depth24 );
  }

bool GraphicsSystem::render( Scene &scene,
                             ParticleSystemEngine & par,
                             Tempest::Camera   camera,
                             size_t dt) {
  if( !device.startRender() )
    return false;

  //return false;
  onRender( std::max<size_t>(dt-time, 0) );

  time = dt;//(time+dt);
  unsigned tx = time%(16*1024);

  Material::wind = sin( 2.0*M_PI*tx/(16*1024.0) );

  particles = &par;

  Tempest::Texture2d gbuffer[4], rsm[4];
  Tempest::Texture2d mainDepth = depth( screenSize );
  scrOffset.set( 1.0f/screenSize.w, 1.0f/screenSize.h );

  for( int i=0; i<3; ++i ){
    gbuffer[i] = colorBuf( screenSize.w, screenSize.h );
    }

  if( useHDR )
    gbuffer[3] = localTex.create( screenSize.w, screenSize.h,
                                  Tempest::Texture2d::Format::RGBA16 ); else
    gbuffer[3] = localTex.create( screenSize.w, screenSize.h,
                                  Tempest::Texture2d::Format::RG16 );
  //buildRSM( scene, rsm, 512, 0 );
  scene.setCamera( camera );
  /*
  blt( rsm[0] );
  device.present();
  return 1;
  */
/*
  renderScene( scene,
               scene.camera(),
               gbuffer, mainDepth, rsm,
               2048, true );
*/
  Tempest::Texture2d fog;
  /*
  drawFogOfWar(fog, scene);
  aceptFog( gbuffer[0], fog );
*/
  if( widget )
    gui->exec( *widget, gbuffer[0], mainDepth, device );

  Tempest::Texture2d glow, bloomTex;
  //drawGlow( glow, mainDepth, scene, 512 );
  //aceptFog( glow, fog );

  bloom( bloomTex, gbuffer[0] );
  //blt( glow );
  //blt( bloomTex );

  Tempest::Texture2d final = localTex.create( screenSize.w, screenSize.h );
  { final.setSampler( reflect );

    Tempest::Texture2d depth = this->depth( final.width(),
                                            final.height() );

    Tempest::Render render( device,
                            final, depth,
                            finalData.vs, finalData.fs );

    render.setRenderState( Tempest::RenderState::PostProcess );

    cpyOffset.set( 1.0/final.width(), 1.0/final.height() );
    device.setUniform( finalData.vs, cpyOffset );

    finalData.bloom.set( &bloomTex );
    finalData.scene.set( &gbuffer[0] );
    finalData.glow .set( &glow );

    device.setUniform( finalData.fs, finalData.bloom );
    device.setUniform( finalData.fs, finalData.scene );
    device.setUniform( finalData.fs, finalData.glow  );

    ppHelper.drawFullScreenQuad( device, finalData.vs, finalData.fs );
    }

  //blt( gbuffer[2] );
  //blt( mainDepth );
  //blt( waterWaves );
//  blt( shadowMap );
  //blt( fog );
  blt( final );
  //blt( gao );
  //blt( ssaoTexDet );
  //blt( bloomTex );

  device.present();
  return 1;
  }

void GraphicsSystem::fillShadowMap( Tempest::Texture2d& shadowMap,
                                    const Scene & scene ) {
  Tempest::Texture2d depthSm = depth( shadowMap.width(), shadowMap.height() );

  fillShadowMap( shadowMap, depthSm, scene,
                 scene.shadowCasters() );
  }

void GraphicsSystem::fillShadowMap( Tempest::Texture2d& sm,
                                    Tempest::Texture2d& depthSm,
                                    const Scene & scene,
                                    const Scene::Objects &v ) {
  Tempest::RenderState rstate;
  rstate.setCullFaceMode( Tempest::RenderState::CullMode::front );

  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };
  Tempest::Matrix4x4 matrix = makeShadowMatrix(scene, dir);

  Tempest::Render render( device,
                          sm, depthSm,
                          smap.vs, smap.fs );
  {
    render.clear( Tempest::Color(1.0), 1 );
    if( !(sm.width()==1 && sm.height()==1) ){
      Tempest::Matrix4x4 proj;
      proj.identity();

      particles->exec( matrix, proj, 0, true );
      }
    }

  if( sm.width()==1 && sm.height()==1 )
    return;

  render.setRenderState( rstate );
  //return;

  const Tempest::AbstractCamera & camera = scene.camera();

  Frustum frustum;
  mkFrustum( matrix, frustum );

  draw( render,
        frustum,
        true,
        camera,
        v,
        &Material::shadow,
        ref(matrix) );
  }

void GraphicsSystem::fillTranscurentMap( Tempest::Texture2d &sm,
                                         Tempest::Texture2d &depthSm,
                                         const Scene &scene) {

  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  Tempest::Texture2d lightColor = localTex.create(1,1);
  {
    Tempest::Texture2d depthSm = depth(1,1);
    Tempest::Render render( device,
                         sm, depthSm,
                         transparentData.vsSh,
                         transparentData.fsSh );
    render.clear( light.color(), 1 );
    }

  Tempest::RenderState rstate;
  rstate.setCullFaceMode( Tempest::RenderState::CullMode::front );

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };
  Tempest::Render render( device,
                       sm, depthSm,
                       transparentData.vsSh,
                       transparentData.fsSh );
  render.clear( light.color(), 1 );

  if( sm.width()==1 && sm.height()==1 )
    return;

  render.setRenderState( rstate );
  RSMCamera c;
  c.v = makeShadowMatrix( scene, dir );
  c.p.setData( 1, 0, 0, 0,
               0,-1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1 );
/*
  setupLight( scene, transparentData.fsSh, sm, lightColor );
  drawObjects( transparentData.vs,
               transparentData.fsSh,
               &sm, depthSm, 1,
               scene,
               c,
               scene.transparentObjects(),
               &Material::transparentZ,
               false );

  drawObjects( transparentData.vs,
               transparentData.fsSh,
               &sm, depthSm, 1,
               scene,
               c,
               scene.transparentObjects(),
               &Material::transparent,
               false );
               */
  }

void GraphicsSystem::setupLight( const Scene & scene,
                                 Tempest::FragmentShader & fs,
                                 const Tempest::Texture2d & sm,
                                 const Tempest::Texture2d & smCl ){
  if( scene.lights().direction().size()>0 ){
    Tempest::DirectionLight l = scene.lights().direction()[0];

    gbuf.lightDirection.set( l, Tempest::Direction );
    device.setUniform( fs, gbuf.lightDirection );

    gbuf.lightColor.set( l, Tempest::LightColor );
    device.setUniform( fs, gbuf.lightColor );

    gbuf.lightAblimient.set( l, Tempest::LightAblimient );
    device.setUniform( fs, gbuf.lightAblimient );

    Tempest::Matrix4x4 vm = scene.camera().view();

    float view[3] = { float(vm.at(2,0)),
                      float(vm.at(2,1)),
                      float(vm.at(2,2)) };
    float len = sqrt(view[0]*view[0] + view[1]*view[1] + view[2]*view[2]);

    for( int i=0; i<3; ++i )
      view[i] /= len;

    gbuf.view.set( view );
    device.setUniform( fs, gbuf.view );

    device.setUniform( fs,     sm, "shadowMap"    );
    device.setUniform( fs,   smCl, "shadowMapCl"  );
    }
  }

void GraphicsSystem::fillGBuf( Tempest::Texture2d* gbuffer,
                               Tempest::Texture2d& mainDepth,
                               const Tempest::Texture2d& sm,
                               const Tempest::Texture2d& smCl,
                               const Scene & scene,
                               const Tempest::AbstractCamera & camera ) {
  setupLight( scene, gbuf.terrainFs, sm, smCl );
  particles->exec( scene.camera().view(),
                   scene.camera().projective(),
                   1 );

  setupLight( scene, gbuf.fs, sm, smCl );

  drawObjects( gbuf.terrainVs,
               gbuf.terrainFs,
               gbuffer,
               mainDepth,
               4,
               scene,
               camera,
               scene.terrainMinorObjects(),
               &Material::terrainMinorZ,
               true,
               true );
  {
    Tempest::Texture2d d = depth( mainDepth.width(), mainDepth.height() );
    Tempest::Render render( device,
                         gbuffer, 4,
                         d,
                         gbuf.terrainVs, gbuf.terrainFs );
    render.clear( Tempest::Color(0) );
  }


  drawObjects( gbuf.terrainVs,
               gbuf.terrainFs,
               gbuffer,
               mainDepth,
               4,
               scene,
               camera,
               scene.terrainMinorObjects(),
               &Material::terrainMinor,
               false );

  setupLight( scene, gbuf.fs, sm, smCl );

  drawObjects( gbuffer,
               mainDepth,
               4,
               scene,
               camera,
               scene.mainObjects(),
               &Material::gbuffer,
               false );

  setupLight( scene, gbuf.grassFs, sm, smCl );
  drawObjects( gbuf.grassVs,
               gbuf.grassFs,
               gbuffer,
               mainDepth,
               4,
               scene,
               camera,
               scene.grassObjects(),
               &Material::grass );

  drawObjects( transparentData.vsAdd, transparentData.fsAdd,
               gbuffer,
               mainDepth,
               1,
               scene,
               camera,
               scene.additiveObjects(),
               &Material::additive );

  setupLight( scene, transparentData.fs, sm, smCl );
  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer,
               mainDepth,
               1,
               scene,
               camera,
               scene.transparentObjects(),
               &Material::transparentZ,
               false );

  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer,
               mainDepth,
               1,
               scene,
               camera,
               scene.transparentObjects(),
               &Material::transparent,
               false );
  }

void GraphicsSystem::renderVolumeLight( const Scene &scene,
                                        Tempest::Texture2d &gbuffer,
                                        Tempest::Texture2d &mainDepth,
                                        Tempest::Texture2d &shadowMap ) {
  int w = 512,//gbuffer.width(),
      h = 512;//gbuffer.height();

  Tempest::Texture2d d = depth( w, h );
  Tempest::Texture2d vlTex = localTex.create(w, h,
                                          Tempest::AbstractTexture::Format::RGBA );

  { Tempest::Render render( device,
                         vlTex,
                         d,
                         volumetricData.vs,
                         volumetricData.fs );
    render.setRenderState( Tempest::RenderState::PostProcess );

    Tempest::Matrix4x4 mat = scene.camera().projective();
    mat.mul( scene.camera().view() );
    mat.inverse();

    Tempest::Matrix4x4 smM = closure.shadow.matrix;

    device.setUniform( volumetricData.fs, mat, "invMatrix");
    device.setUniform( volumetricData.fs, smM,  "shMatrix");
    device.setUniform( volumetricData.fs, shadowMap, "shadowMap");
    device.setUniform( volumetricData.fs, mainDepth, "mainDepth");

    cpyOffset.set( 1.0f/vlTex.width(), 1.0f/vlTex.height() );
    device.setUniform( volumetricData.vs, cpyOffset );

    ppHelper.drawFullScreenQuad( device, volumetricData.vs, volumetricData.fs );
    }

  { Tempest::Texture2d depth = this->depth( mainDepth.width(),
                                         mainDepth.height() );

    Tempest::Render render( device,
                         gbuffer, depth,
                         bltData.vs, bltData.fs );

    Tempest::RenderState rs = Tempest::RenderState::PostProcess;
    rs.setBlend(1);
    /*
    rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::dst_color,
                     Tempest::RenderState::AlphaBlendMode::zero );*/
    rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::src_alpha,
                     Tempest::RenderState::AlphaBlendMode::one_minus_src_alpha );

    render.setRenderState( rs );

    bltData.texture.set( &vlTex );
    cpyOffset.set( 1.0f/gbuffer.width(), 1.0f/gbuffer.height() );
    device.setUniform( bltData.vs, cpyOffset );
    device.setUniform( bltData.fs, bltData.texture );

    ppHelper.drawFullScreenQuad( device, bltData.vs, bltData.fs );
    }
  }

void GraphicsSystem::drawOmni( Tempest::Texture2d *gbuffer,
                               Tempest::Texture2d &mainDepth,
                               Tempest::Texture2d & sm,
                               const Scene &scene ) {
  Tempest::Render render( device,
                       gbuffer[0],
                       mainDepth,
                       omniData.vs,
                       omniData.fs );

  device.setUniform( omniData.fs,
                     gbuffer[3],
                     "texture" );

  device.setUniform( omniData.fs,
                     gbuffer[1],
                     "albedo" );

  device.setUniform( omniData.fs,
                     gbuffer[2],
                     "normals" );

  device.setUniform( omniData.fs,
                     sm,
                     "shadowMap" );

  double dir[3] = {0,0,-1};
  Tempest::Matrix4x4 shM = makeShadowMatrix(scene,dir);


  float tc[] = { 1.0f/gbuffer[3].width(), 1.0f/gbuffer[3].height() };
  device.setUniform( omniData.fs, tc, 2, "dTexCoord");

  const Tempest::AbstractCamera & camera = scene.camera();
  const Scene::Objects &v = scene.omni();

  Frustum frustum;
  mkFrustum( camera, frustum );

  draw( render,
        frustum,
        true,
        camera,
        v,
        &Material::omni,
        shM );
  }

void GraphicsSystem::drawObjects( Tempest::Texture2d* gbuffer,
                                  Tempest::Texture2d& mainDepth,
                                  int bufC,
                                  const Scene &scene,
                                  const Tempest::AbstractCamera & camera,
                                  const Scene::Objects &v,
                                  void (Material::*func)( Tempest::RenderState& /*d*/,
                                                          const Tempest::Matrix4x4 & /*object*/,
                                                          const Tempest::AbstractCamera&,
                                                          Tempest::UniformTable &,
                                                          const Tempest::Matrix4x4 & ) const,
                                  bool clr ) {
  drawObjects( gbuf.vs, gbuf.fs, gbuffer, mainDepth,
               bufC, scene, camera, v, func, clr );
  }

void GraphicsSystem::drawObjects( Tempest::VertexShader   & vs,
                                  Tempest::FragmentShader & fs,
                                  Tempest::Texture2d* gbuffer,
                                  Tempest::Texture2d& mainDepth,
                                  int bufC,
                                  const Scene &scene,
                                  const Tempest::AbstractCamera& camera,
                                  const Scene::Objects &v,
                                  void (Material::*func)( Tempest::RenderState& /*d*/,
                                                          const Tempest::Matrix4x4 & /*object*/,
                                                          const Tempest::AbstractCamera&,
                                                          Tempest::UniformTable &,
                                                          const Tempest::Matrix4x4 & ) const,
                                  bool clr,
                                  bool clrDepth ) {
  Tempest::Render render( device,
                       gbuffer,
                       bufC,
                       mainDepth,
                       vs, fs );
  if( clr ){
    if( clrDepth )
      render.clear( Tempest::Color(0.0), 1 ); else
      render.clear( Tempest::Color(0.0) );
    }

  Tempest::Matrix4x4 matrix = makeShadowMatrix(scene);

  Frustum frustum;
  mkFrustum( camera, frustum );
  draw( render, frustum, true, camera, v, func, ref(matrix) );
  }

void GraphicsSystem::drawTranscurent( Tempest::Texture2d& screen,
                                      Tempest::Texture2d& mainDepth,
                                      Tempest::Texture2d& sceneCopy,
                                      const Scene &scene,
                                      const Scene::Objects &v ) {
  Tempest::Render render( device,
                       screen,
                       mainDepth,
                       displaceData.vs,
                       displaceData.fs );

  device.setUniform( displaceData.fs,
                     sceneCopy,
                     "scene" );
  float tc[] = { 1.0f/sceneCopy.width(), 1.0f/sceneCopy.height() };
  device.setUniform( displaceData.fs, tc, 2, "dTexCoord");

  const Tempest::AbstractCamera & camera = scene.camera();
  Tempest::Matrix4x4 matrix = makeShadowMatrix(scene);

  Frustum frustum;
  mkFrustum( camera, frustum );
  draw( render, frustum, true, camera, v, &Material::displace, ref(matrix) );
  }

template< class ... Args, class ... FArgs >
void GraphicsSystem::draw( Tempest::Render  & render,
                           const Frustum &frustum,
                           bool deepVTest,
                           const Tempest::AbstractCamera & camera,
                           const Scene::Objects & v,
                           void (Material::*func)( Tempest::RenderState& /*d*/,
                                                   const Tempest::Matrix4x4 & /*object*/,
                                                   const Tempest::AbstractCamera&,
                                                   Tempest::UniformTable &,
                                                   FArgs ... args ) const,
                           Args... args ){
  for( size_t i=0; i<v.objects.size(); ++i ){
    const AbstractGraphicObject& ptr = *v.objects[i];

    if( !deepVTest || isVisible(ptr,frustum) ){
      Tempest::UniformTable table( render );
      Tempest::RenderState rs;

      (ptr.material().*func)( rs, ptr.transform(), camera, table, args... );
      device.setRenderState(rs);
      render.draw( ptr );
      }
    }

  for( int x=0; x<2; ++x )
    for( int y=0; y<2; ++y )
      for( int z=0; z<2; ++z )
        if( v.nested[x][y][z] ){
          Scene::Objects &t = *v.nested[x][y][z];
          float dpos = t.linearSize*0.5;

          GraphicsSystem::VisibleRet ret = FullVisible;
          if( deepVTest )
            ret = isVisible( t.x+dpos, t.y+dpos, t.z+dpos, t.r, frustum );

          if( ret ){
            draw( render, frustum, ret!=FullVisible, camera, t, func, args... );
            }
          }
  }

void GraphicsSystem::drawWater( Tempest::Texture2d& screen,
                                Tempest::Texture2d& mainDepth,
                                Tempest::Texture2d& sceneCopy,
                                Tempest::Texture2d& sm,
                                Tempest::Texture2d& smCl,
                                Tempest::Texture2d& sceneDepth,
                                const Scene &scene,
                                const Scene::Objects &v ) {

  Tempest::Texture2d waterWaves;
  waves( waterWaves, water.waterHeightMap[0], water.waterHeightMap[1] );

  setupLight( scene, displaceData.fsWater, sm, smCl );
  Tempest::Render render( device,
                       screen,
                       mainDepth,
                       displaceData.vsWater,
                       displaceData.fsWater );

  device.setUniform( displaceData.fsWater,
                     sceneCopy,
                     "scene" );
  device.setUniform( displaceData.fsWater,
                     water.envMap,
                     "envMap" );

  float tc[] = { 1.0f/sceneCopy.width(), 1.0f/sceneCopy.height() };
  device.setUniform( displaceData.fsWater, tc, 2, "dTexCoord");
  device.setUniform( displaceData.fsWater, waterWaves, "normalMap");
  device.setUniform( displaceData.fsWater, sceneDepth, "sceneDepth");

  float dc[] = {0,0};
  device.setUniform( displaceData.fsWater, dc, 2, "dWaterCoord");

  const Tempest::AbstractCamera & camera = scene.camera();
  Tempest::Matrix4x4 matrix = makeShadowMatrix(scene);

  Frustum frustum;
  mkFrustum( camera, frustum );

  draw( render,
        frustum,
        true,
        camera,
        v,
        &Material::water,
        ref(matrix) );
  }

void GraphicsSystem::drawGlow (Tempest::Texture2d &out,
                               Tempest::Texture2d &depth,
                               const Scene &scene,
                               int size ) {
  Tempest::Texture2d buffer = colorBuf( depth.width(),
                                     depth.height() );

  const Tempest::AbstractCamera & camera = scene.camera();
  drawObjects( glowData.vs, glowData.fs,
               &buffer,
               depth,
               1,
               scene,
               camera,
               scene.glowObjects(),
               &Material::glowPass,
               true,
               false );

  //int size = 512;
  Tempest::Texture2d tmp;
  copy ( tmp, buffer,  size, size );
  gauss( buffer, tmp,  size, size, 1.0, 0.0 );
  gauss( out,  buffer, size, size, 0.0, 1.0 );

  //out = tmp;
  }

void GraphicsSystem::blt( const Tempest::Texture2d & tex ) {
  Tempest::Render render( device, bltData.vs, bltData.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  bltData.texture.set( &tex );
  device.setUniform( bltData.vs, scrOffset );
  device.setUniform( bltData.fs, bltData.texture );

  ppHelper.drawFullScreenQuad( device, bltData.vs, bltData.fs );
  }

void GraphicsSystem::copy( Tempest::Texture2d &out,
                           const Tempest::Texture2d& in ) {
  copy(out, in, in.width(), in.height() );
  }

void GraphicsSystem::copy( Tempest::Texture2d &out,
                           const Tempest::Texture2d& in,
                           int w, int h ) {
  out = colorBuf( w,h );
  out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                       out, depth,
                       bltData.vs, bltData.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( bltData.vs, cpyOffset );
  device.setUniform( bltData.fs, bltData.texture );

  ppHelper.drawFullScreenQuad( device, bltData.vs, bltData.fs );
  }

void GraphicsSystem::copyDepth( Tempest::Texture2d &out,
                                const Tempest::Texture2d& in,
                                int w, int h ) {
  //int w = in.width(), h = in.height();

  out = localTex.create( w, h, Tempest::Texture2d::Format::RG16 );
  out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                       out, depth,
                       bltData.vs, bltData.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( bltData.vs, cpyOffset );
  device.setUniform( bltData.fs, bltData.texture );

  ppHelper.drawFullScreenQuad( device, bltData.vs, bltData.fs );
  }

void GraphicsSystem::gauss( Tempest::Texture2d &out,
                            const Tempest::Texture2d& in,
                            int w, int h,
                            float dx, float dy ) {
  out = colorBuf( w,h );
  out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                          out, depth,
                          gaussData.vs, gaussData.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( gaussData.vs, cpyOffset );

  //cpyOffset.set( dx/w, dy/h );
  float blurCoord[2] = { dx/w, dy/h };
  device.setUniform( gaussData.fs, blurCoord, 2, "blurCoord" );
  device.setUniform( gaussData.fs, bltData.texture );

  ppHelper.drawFullScreenQuad( device, gaussData.vs, gaussData.fs );
  }

void GraphicsSystem::gauss_gb( Tempest::Texture2d &out,
                               const Tempest::Texture2d& in,
                               int w, int h,
                               float dx, float dy ) {
  out = localTex.create( w,h );
  out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                       out, depth,
                       gaussData.vsGB, gaussData.fsGB );

  render.setRenderState( Tempest::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( gaussData.vsGB, cpyOffset );

  cpyOffset.set( dx/w, dy/h );
  device.setUniform( gaussData.fsGB, cpyOffset );
  device.setUniform( gaussData.fsGB, bltData.texture );

  ppHelper.drawFullScreenQuad( device, gaussData.vsGB, gaussData.fsGB );
  }

void GraphicsSystem::gauss_b( Tempest::Texture2d &out,
                               const Tempest::Texture2d& in,
                               int w, int h,
                               float dx, float dy ) {
  out = localTex.create( w,h );
  out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                       out, depth,
                       gaussData.vsB, gaussData.fsB );

  render.setRenderState( Tempest::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( gaussData.vsB, cpyOffset );

  cpyOffset.set( dx/w, dy/h );
  device.setUniform( gaussData.fsB, cpyOffset );
  device.setUniform( gaussData.fsB, bltData.texture );

  ppHelper.drawFullScreenQuad( device, gaussData.vsB, gaussData.fsB );
  }

void GraphicsSystem::bloom( Tempest::Texture2d &result,
                            const Tempest::Texture2d &in ) {
  //bloomData
  const int w = 256, h = w;

  result = colorBuf( w,h );
  Tempest::Texture2d tmp[4];

  Tempest::Size sizes[3] = {
    {256, 256}, {64, 64}, {16, 16}
    };

  {
    tmp[0] = colorBuf( w,h );
    tmp[0].setSampler( reflect );
    Tempest::Texture2d depth = this->depth( w,h );

    Tempest::Render render( device,
                            tmp[0], depth,
                            bloomData.vs, bloomData.brightPass );

    render.setRenderState( Tempest::RenderState::PostProcess );

    bltData.texture.set( &in );
    cpyOffset.set( 1.0f/w, 1.0f/h );
    device.setUniform( bloomData.vs, cpyOffset );

    device.setUniform( bloomData.brightPass, bltData.texture );

    ppHelper.drawFullScreenQuad( device, bloomData.vs, bloomData.brightPass );
    }

  for( int i=1; i<4; ++i ){
    Tempest::Texture2d & out = tmp[i];

    const Tempest::Size & sz = sizes[i-1];
    out = colorBuf( sz.w, sz.h );
    out.setSampler( reflect );

    Tempest::Texture2d htmp;

    gauss( htmp, tmp[i-1], sz.w, sz.h, 1.0, 0.0 );
    gauss(  out,     htmp, sz.w, sz.h, 0.0, 1.0 );
    }

  { result.setSampler( reflect );

    Tempest::Texture2d depth = this->depth( result.width(),
                                         result.height() );

    Tempest::Render render( device,
                            result, depth,
                            bloomData.vs, bloomData.brightPass );

    render.setRenderState( Tempest::RenderState::PostProcess );

    cpyOffset.set( 0,0 );
    device.setUniform( bloomData.vs, cpyOffset );

    for( int i=0; i<3; ++i ){
      bloomData.b[i].set( &tmp[i+1] );
      device.setUniform( bloomData.combine, bloomData.b[i] );
      }

    ppHelper.drawFullScreenQuad( device, bloomData.vs, bloomData.combine );
    }
  }

void GraphicsSystem::drawFogOfWar( Tempest::Texture2d &out,
                                   const Scene & scene ) {
  int size = 256;

  Tempest::Texture2d depth  = this->depth( size, size );
  Tempest::Texture2d buffer = localTex.create( depth.width(),
                                            depth.height(),
                                            Tempest::Texture2d::Format::RGBA );

  {
    Tempest::Render render( device,
                         buffer,
                         depth,
                         fogOfWar.vs,
                         fogOfWar.fs );
    render.clear( Tempest::Color(0,0,0, 1), 1 );
    device.setUniform( fogOfWar.fs, fogView, "texture" );

    const Tempest::AbstractCamera & camera = scene.camera();

    const Scene::Objects & v = scene.fogOfWar();

    Frustum frustum;
    mkFrustum( camera, frustum );
    draw( render, frustum, true, camera, v, &Material::fogOfWar, true );
    draw( render, frustum, true, camera, v, &Material::fogOfWar, false );
    }

  Tempest::Texture2d tmp;
  copy ( tmp, buffer,  size, size );
  gauss( buffer, tmp,  size, size, 2.0, 0.0 );
  gauss( out,  buffer, size, size, 0.0, 2.0 );
  }

void GraphicsSystem::aceptFog( Tempest::Texture2d &in_out,
                               const Tempest::Texture2d &fog ) {
  if( !(fogView.width()>1 || fogView.height()>1 ) )
    return;

  Tempest::Texture2d depth = this->depth( in_out.width(),
                                       in_out.height() );
  Tempest::Texture2d tmp;
  copy(tmp, in_out);

  Tempest::Render render( device,
                       in_out, depth,
                       fogOfWar.vsAcept,
                       fogOfWar.fsAcept );

  render.setRenderState( Tempest::RenderState::PostProcess );

  cpyOffset.set( 1.0/tmp.width(), 1.0/tmp.height() );
  device.setUniform( fogOfWar.vsAcept, cpyOffset );

  device.setUniform( fogOfWar.fsAcept, tmp, "scene" );
  device.setUniform( fogOfWar.fsAcept, fog, "fog"   );

  ppHelper.drawFullScreenQuad( device, fogOfWar.vsAcept, fogOfWar.fsAcept );
  }

void GraphicsSystem::waves( Tempest::Texture2d &out,
                            const Tempest::Texture2d& in,
                            const Tempest::Texture2d& in1 ) {
  int w = 2*in.width(), h = 2*in.height();

  out = localTex.create( w,h );
  //out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth(w,h);
  Tempest::Render render( device,
                       out, depth,
                       water.vs, water.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( water.vs, cpyOffset );

  device.setUniform( water.fs, bltData.texture );
  device.setUniform( water.fs, in1, "texture1" );

  //sin( 2.0*M_PI*time/(4*1024.0) )
  //static unsigned time = 0;
  //++time;
  float t = 2*M_PI*float(time%2048)/2048.0f;

  device.setUniform ( water.fs, &t, 1, "time" );

  ppHelper.drawFullScreenQuad( device, water.vs, water.fs );
  }

void GraphicsSystem::ssaoDetail( Tempest::Texture2d &out,
                                 const Tempest::Texture2d &in ,
                                 const Tempest::Texture2d &macro ) {
  int w = 512, h = w;

  Tempest::Texture2d tmp;
  copy( tmp, in, w, h );
  gauss( out, tmp, w, h, 2, 0 );
  gauss( tmp, out, w, h, 0, 2 );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                       out, depth,
                       ssaoData.vs, ssaoData.detail );

  render.setRenderState( Tempest::RenderState::PostProcess );

  ssaoData.texture.set( &in );
  ssaoData.blured .set( &tmp );
  ssaoData.macro  .set( &macro );

  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( ssaoData.vs, cpyOffset );
  device.setUniform( ssaoData.detail, ssaoData.texture );
  device.setUniform( ssaoData.detail, ssaoData.blured  );
  device.setUniform( ssaoData.detail, ssaoData.macro  );

  ppHelper.drawFullScreenQuad( device, ssaoData.vs, ssaoData.detail );
  }

void GraphicsSystem::ssao( Tempest::Texture2d &out,
                           const Tempest::Texture2d &in,
                           const Tempest::Texture2d & gao,
                           const Scene & scene ) {
  int w = in.width(), h = in.height();

  const Tempest::AbstractCamera &camera = scene.camera();

  Tempest::Matrix4x4 mat = camera.projective();
  mat.mul( camera.view() );
  mat.inverse();

  double dir[] = {0,0, -1};
  Tempest::Matrix4x4 shMatrix = makeShadowMatrix(scene, dir);

  const Tempest::Camera &view =
      reinterpret_cast<const Tempest::Camera&>( scene.camera() );
  float scaleSize = 0.3/std::max( view.distance(), 1.0 )/3.0;

  out = localTex.create( w, h, Tempest::Texture2d::Format::Luminance8 );
  out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                       out, depth,
                       ssaoData.vs, ssaoData.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  ssaoData.texture.set( &gao );
  cpyOffset.set( 1.0f/screenSize.w, 1.0f/screenSize.h );
  device.setUniform( ssaoData.vs, cpyOffset );
  device.setUniform( ssaoData.fs, ssaoData.texture );
  device.setUniform( ssaoData.fs, mat,      "invMatrix");
  device.setUniform( ssaoData.fs, shMatrix, "shMatrix" );
  device.setUniform( ssaoData.fs, in, "dBuf" );
  device.setUniform( ssaoData.fs, &scaleSize, 1, "scaleSize" );

  ppHelper.drawFullScreenQuad( device, ssaoData.vs, ssaoData.fs );
  }

void GraphicsSystem::aceptGI(   const Scene & s,
                                Tempest::Texture2d &out,
                                const Tempest::Texture2d &scene,
                                const Tempest::Texture2d &diff,
                                const Tempest::Texture2d &norm,
                                const Tempest::Texture2d &sdepth,
                                const Tempest::Texture2d gi[4] ) {
  //ssaoData.lightAblimient.set(1,1,1);

  if( s.lights().direction().size()>0 ){
    Tempest::DirectionLight l = s.lights().direction()[0];
    ssaoData.lightAblimient.set( l, Tempest::LightAblimient );
    }

  int w = scene.width(), h = scene.height();

  out = colorBuf( w,h );
  out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                       out, depth,
                       ssaoData.vs, ssaoData.accept );

  render.setRenderState( Tempest::RenderState::PostProcess );

  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( ssaoData.vs, cpyOffset );

  const Tempest::AbstractCamera &camera = s.camera();
  Tempest::Matrix4x4 mat = camera.projective();
  mat.mul( camera.view() );
  mat.inverse();

  Tempest::DirectionLight light;
  if( s.lights().direction().size() > 0 )
    light = s.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };
  Tempest::Matrix4x4 shMatrix = makeShadowMatrix(s, dir);

  device.setUniform( ssaoData.acceptGI, mat,      "invMatrix" );
  device.setUniform( ssaoData.acceptGI, shMatrix, "shMatrix"  );
  shMatrix.inverse();
  device.setUniform( ssaoData.acceptGI, shMatrix, "invShMatrix"  );
  device.setUniform( ssaoData.acceptGI, sdepth,   "depth"     );

  ssaoData.scene.set( &scene );
  ssaoData.diff .set( &diff  );
  ssaoData.ssao .set( &gi[0]  );
  device.setUniform( ssaoData.acceptGI, gi[2],   "ssaoN"    );
  device.setUniform( ssaoData.acceptGI, gi[3],   "ssaoD"     );
  device.setUniform( ssaoData.acceptGI, ssaoData.scene );
  device.setUniform( ssaoData.acceptGI, ssaoData.diff  );
  device.setUniform( ssaoData.acceptGI, ssaoData.ssao  );
  device.setUniform( ssaoData.acceptGI, ssaoData.lightAblimient  );

  ppHelper.drawFullScreenQuad( device, ssaoData.vs, ssaoData.acceptGI );
  }

void GraphicsSystem::aceptSsao( const Scene & s,
                                Tempest::Texture2d &out,
                                const Tempest::Texture2d &scene,
                                const Tempest::Texture2d &diff,
                                const Tempest::Texture2d &ssao ) {
  //ssaoData.lightAblimient.set(1,1,1);

  if( s.lights().direction().size()>0 ){
    Tempest::DirectionLight l = s.lights().direction()[0];
    ssaoData.lightAblimient.set( l, Tempest::LightAblimient );
    }

  int w = scene.width(), h = scene.height();

  out = colorBuf( w,h );
  out.setSampler( reflect );

  Tempest::Texture2d depth = this->depth( w,h );

  Tempest::Render render( device,
                       out, depth,
                       ssaoData.vs, ssaoData.accept );

  render.setRenderState( Tempest::RenderState::PostProcess );

  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( ssaoData.vs, cpyOffset );

  ssaoData.scene.set( &scene );
  ssaoData.diff .set( &diff  );
  ssaoData.ssao .set( &ssao  );
  device.setUniform( ssaoData.accept, ssaoData.scene );
  device.setUniform( ssaoData.accept, ssaoData.diff  );
  device.setUniform( ssaoData.accept, ssaoData.ssao  );
  device.setUniform( ssaoData.accept, ssaoData.lightAblimient  );

  ppHelper.drawFullScreenQuad( device, ssaoData.vs, ssaoData.accept );
  }

void GraphicsSystem::ssaoGMap( const Scene &scene,
                               Tempest::Texture2d &sm  ) {
  sm = localTex.create( 256, 256,
                        Tempest::AbstractTexture::Format::RGB10_A2 );

  Tempest::RenderState rstate;
  rstate.setCullFaceMode( Tempest::RenderState::CullMode::front );

  Tempest::Texture2d depthSm = depth( sm.width(), sm.height() );
  { const Scene::Objects &v = scene.shadowCasters();
    Tempest::Render render( device,
                         sm, depthSm,
                         smap.vs, smap.fs );
    render.clear( Tempest::Color(1.0), 1 );
    render.setRenderState( rstate );

    const Tempest::AbstractCamera & camera = scene.camera();

    double dir[] = {0,0, -1};
    Tempest::Matrix4x4 matrix = makeShadowMatrix(scene, dir);

    Frustum frustum;
    mkFrustum( matrix, frustum );

    draw( render,
          frustum,
          true,
          camera,
          v,
          &Material::shadow,
          ref(matrix) );
    }
  }

Tempest::Texture2d GraphicsSystem::colorBuf(int w, int h) {
  if( useHDR )
    return localTex.create( w,h, Tempest::Texture2d::Format::RGBA16 ); else
    return localTex.create( w,h, Tempest::Texture2d::Format::RGBA8 );
  }

void GraphicsSystem::blurSm( Tempest::Texture2d &sm,
                             const Scene & scene ) {
  Tempest::Texture2d tmp = localTex.create( sm.width(), sm.height(),
                        Tempest::AbstractTexture::Format::RGB10_A2 );

  float s = 6*smMatSize(scene);
  gauss( tmp,  sm, sm.width(), sm.height(), s, 0 );
  gauss(  sm, tmp, sm.width(), sm.height(), 0, s );

  gauss_gb( tmp,  sm, sm.width(), sm.height(), s*3, 0 );
  gauss_gb(  sm, tmp, sm.width(), sm.height(), 0, s*3 );

  gauss_b( tmp,  sm, sm.width(), sm.height(),  s*4, 0 );
  gauss_b(  sm, tmp, sm.width(), sm.height(),  0, s*4 );

  Tempest::Texture2d::Sampler sampler = reflect;
  sampler.uClamp = Tempest::Texture2d::ClampMode::Clamp;
  sampler.vClamp = sampler.uClamp;

  sm.setSampler( sampler );
  }

void GraphicsSystem::buildRSM( Scene &scene,
                               Tempest::Texture2d gbuffer[4],
                               int shadowMapSize ) {
  for( int i=0; i<3; ++i ){
    gbuffer[i] = localTex.create( shadowMapSize, shadowMapSize,
                                  Tempest::Texture2d::Format::RGBA );
    }
  gbuffer[3] = localTex.create( shadowMapSize, shadowMapSize,
                                Tempest::Texture2d::Format::RG16 );
  Tempest::Texture2d mainDepth = depth( shadowMapSize, shadowMapSize );

  //Tempest::Camera &c = scene.camera();

  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };
  RSMCamera c;
  c.v = makeShadowMatrix( scene, dir );
  c.p.setData( 1, 0, 0, 0,
               0,-1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1 );

  // scene.setCamera( c );

  Tempest::Texture2d rsmNo[4];
  renderScene( scene,
               c,
               gbuffer,
               mainDepth, rsmNo, 1, 0 );
  }

void GraphicsSystem::mkFrustum( const Tempest::AbstractCamera &c,
                                GraphicsSystem::Frustum    &out ) {
  Tempest::Matrix4x4 cl = c.projective();
  cl.mul( c.view() );
  mkFrustum(cl, out);
  }

void GraphicsSystem::mkFrustum( const Tempest::Matrix4x4 &cl,
                                GraphicsSystem::Frustum    &out ) {
  float clip[16], t;
  std::copy( cl.data(), cl.data()+16, clip );

  out.f[0][0] = clip[ 3] - clip[ 0];
  out.f[0][1] = clip[ 7] - clip[ 4];
  out.f[0][2] = clip[11] - clip[ 8];
  out.f[0][3] = clip[15] - clip[12];

  t = sqrt( out.f[0][0] * out.f[0][0] + out.f[0][1] * out.f[0][1] + out.f[0][2] * out.f[0][2] );

  out.f[0][0] /= t;
  out.f[0][1] /= t;
  out.f[0][2] /= t;
  out.f[0][3] /= t;

  out.f[1][0] = clip[ 3] + clip[ 0];
  out.f[1][1] = clip[ 7] + clip[ 4];
  out.f[1][2] = clip[11] + clip[ 8];
  out.f[1][3] = clip[15] + clip[12];

  t = sqrt( out.f[1][0] * out.f[1][0] + out.f[1][1] * out.f[1][1] + out.f[1][2] * out.f[1][2] );

  out.f[1][0] /= t;
  out.f[1][1] /= t;
  out.f[1][2] /= t;
  out.f[1][3] /= t;

  out.f[2][0] = clip[ 3] + clip[ 1];
  out.f[2][1] = clip[ 7] + clip[ 5];
  out.f[2][2] = clip[11] + clip[ 9];
  out.f[2][3] = clip[15] + clip[13];

  t = sqrt( out.f[2][0] * out.f[2][0] + out.f[2][1] * out.f[2][1] + out.f[2][2] * out.f[2][2] );

  out.f[2][0] /= t;
  out.f[2][1] /= t;
  out.f[2][2] /= t;
  out.f[2][3] /= t;

  out.f[3][0] = clip[ 3] - clip[ 1];
  out.f[3][1] = clip[ 7] - clip[ 5];
  out.f[3][2] = clip[11] - clip[ 9];
  out.f[3][3] = clip[15] - clip[13];

  t = sqrt( out.f[3][0] * out.f[3][0] + out.f[3][1] * out.f[3][1] + out.f[3][2] * out.f[3][2] );

  out.f[3][0] /= t;
  out.f[3][1] /= t;
  out.f[3][2] /= t;
  out.f[3][3] /= t;

  out.f[4][0] = clip[ 3] - clip[ 2];
  out.f[4][1] = clip[ 7] - clip[ 6];
  out.f[4][2] = clip[11] - clip[10];
  out.f[4][3] = clip[15] - clip[14];

  t = sqrt( out.f[4][0] * out.f[4][0] + out.f[4][1] * out.f[4][1] + out.f[4][2] * out.f[4][2] );

  out.f[4][0] /= t;
  out.f[4][1] /= t;
  out.f[4][2] /= t;
  out.f[4][3] /= t;

  out.f[5][0] = clip[ 3] + clip[ 2];
  out.f[5][1] = clip[ 7] + clip[ 6];
  out.f[5][2] = clip[11] + clip[10];
  out.f[5][3] = clip[15] + clip[14];

  t = sqrt( out.f[5][0] * out.f[5][0] + out.f[5][1] * out.f[5][1] + out.f[5][2] * out.f[5][2] );

  out.f[5][0] /= t;
  out.f[5][1] /= t;
  out.f[5][2] /= t;
  out.f[5][3] /= t;
  }

GraphicsSystem::VisibleRet GraphicsSystem::isVisible( const AbstractGraphicObject &c,
                                                      const GraphicsSystem::Frustum &frustum ) {
  if( !c.isVisible() )
    return NotVisible;

  float r = c.radius(),
        x = c.x() + c.bounds().mid[0]*c.sizeX(),
        y = c.y() + c.bounds().mid[1]*c.sizeY(),
        z = c.z() + c.bounds().mid[2]*c.sizeZ();

  return isVisible(x,y,z, r, frustum);
  }

GraphicsSystem::VisibleRet GraphicsSystem::isVisible( float x,
                                                      float y,
                                                      float z,
                                                      float r,
                                                      const GraphicsSystem::Frustum &frustum ) {
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

void GraphicsSystem::renderScene( const Scene &scene,
                                  const Tempest::AbstractCamera & camera,
                                  Tempest::Texture2d gbuffer[4],
                                  Tempest::Texture2d &mainDepth,

                                  Tempest::Texture2d rsm[4],
                                  int shadowMapSize,
                                  bool useAO ) {
  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };

  closure.shadow.matrix = makeShadowMatrix( scene, dir );

  Tempest::Texture2d shadowMap = localTex.create( shadowMapSize,
                                                  shadowMapSize,
                                                  Tempest::AbstractTexture::Format::Luminance16 );
  fillShadowMap( shadowMap, scene );

  Tempest::Texture2d shadowMapCl = localTex.create(shadowMapSize, shadowMapSize),
                     depth = this->depth(shadowMapSize, shadowMapSize);
  fillTranscurentMap(shadowMapCl, depth, scene);

  fillGBuf( gbuffer, mainDepth,
            shadowMap, shadowMapCl,
            scene, camera );

  Tempest::Texture2d topSm;
  ssaoGMap( scene, topSm );

  drawOmni( gbuffer, mainDepth, topSm, scene );

  //blt( shadowMap );

  if( useAO ){
    Tempest::Texture2d ssaoTex;
    blurSm(topSm, scene);

    ssao( ssaoTex, gbuffer[3], topSm, scene );

    Tempest::Texture2d aoAcepted;

    if( rsm[0].width()>0 )
      aceptGI  ( scene, aoAcepted, gbuffer[0], gbuffer[1],
                                   gbuffer[2], gbuffer[3], rsm ); else
      aceptSsao( scene, aoAcepted, gbuffer[0], gbuffer[1], ssaoTex );

    gbuffer[0] = aoAcepted;
    }

  Tempest::Texture2d sceneCopy;
  copy( sceneCopy, gbuffer[0] );

  drawTranscurent( gbuffer[0], mainDepth, sceneCopy,
                   scene,
                   scene.displaceObjects() );

  drawWater( gbuffer[0], mainDepth,
             sceneCopy,
             shadowMap,
             shadowMapCl,
             gbuffer[3],
             scene,
             scene.waterObjects() );
  /*

  if( useFog )
    renderVolumeLight( scene,
                       gbuffer[0],
                       gbuffer[3],
                       shadowMap );

  if(1){
    setupLight( scene, transparentData.fs, shadowMap, shadowMapCl );
    drawObjects( transparentData.vs, transparentData.fs,
                 gbuffer, mainDepth,
                 scene,
                 camera,
                 scene.objects<TransparentMaterialNoZW>() );
    }
*/
  }

void GraphicsSystem::renderSubScene( const Scene &scene,
                                     ParticleSystemEngine &e,
                                     Tempest::Texture2d &out  ) {
  return;

  particles = &e;

  int w = out.width(),
      h = out.height();

  Tempest::Texture2d gbuffer[4], rsm[4];
  Tempest::Texture2d mainDepth = this->depth(w,h);

  scrOffset.set( 1.0f/w, 1.0f/h );

  for( int i=0; i<3; ++i ){
    gbuffer[i] = colorBuf( w, h );
    }

  if( useHDR )
    gbuffer[3] = localTex.create( w, h,
                                  Tempest::Texture2d::Format::RGBA16 ); else
    gbuffer[3] = localTex.create( w, h,
                                  Tempest::Texture2d::Format::RG16 );

  renderScene( scene,
               scene.camera(),
               gbuffer, mainDepth, rsm, 256, 0 );

  Tempest::Texture2d glow;
  drawGlow( glow, mainDepth, scene, 128 );

  { Tempest::Texture2d depth = this->depth( out.width(),
                                         out.height() );

    Tempest::Render render( device,
                            out, depth,
                            finalData.vs, finalData.avatar );

    render.setRenderState( Tempest::RenderState::PostProcess );

    cpyOffset.set( 1.0/out.width(), 1.0/out.height() );
    device.setUniform( finalData.vs, cpyOffset );

    finalData.scene.set( &gbuffer[0] );
    finalData.glow .set( &glow );

    //device.setUniform( finalData.fs, finalData.bloom );
    device.setUniform( finalData.avatar, finalData.scene );
    device.setUniform( finalData.avatar, finalData.glow );

    ppHelper.drawFullScreenQuad( device, finalData.vs, finalData.avatar );
    }

  //copy( out, glow );
  }

void GraphicsSystem::setFog(const Tempest::Pixmap &p) {
  fogView = localTex.create(p,0);
  }
