#include "graphicssystem.h"

#include <MyGL/Model>
#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include <MyGL/Algo/GBufferFillPass>
#include <MyGL/Algo/Blit>
#include <MyGL/MaterialPass>

#include <MyGL/Scene>

#include <iostream>
#include <cmath>

#include <MyGL/GraphicObject>

#include "graphics/guipass.h"
#include "graphics/displacematerial.h"
#include "graphics/glowmaterial.h"
#include "graphics/transparentmaterial.h"
#include "graphics/addmaterial.h"
#include "graphics/watermaterial.h"
#include "graphics/mainmaterial.h"
#include "graphics/omnimaterial.h"
#include "graphics/blushmaterial.h"
#include "graphics/terrainminormaterial.h"

#include "graphics/particlesystemengine.h"

#include <resource.h>

GraphicsSystem::GraphicsSystem( void *hwnd, int w, int h,
                                bool isFullScreen,
                                int smSize = 1024 )
  : device(directx, makeOpt(isFullScreen), hwnd ),
    texHolder( device ),
    localTex ( device ),
    vboHolder( device ),
    iboHolder( device ),
    vsHolder ( device ),
    fsHolder ( device ),

    ppHelper( vboHolder, iboHolder ),
    gui( vsHolder.load("./data/sh/gui.vert"),
         fsHolder.load("./data/sh/gui.frag"),
         vboHolder,
         screenSize  ) {
  widget = 0;
  time = 0;
  }

void GraphicsSystem::makeRenderAlgo( Resource &res,
                                     MainGui &gui,
                                     int w, int h ) {
    screenSize.w = w;
    screenSize.h = h;

    glowData.vs = res.vshader("glow");
    glowData.fs = res.fshader("glow");

    gbuf.vs     = res.vshader("unit_main_material");
    gbuf.fs     = res.fshader("unit_main_material");

    gbuf.terrainVs = res.vshader("terrain_minor_main_material");
    gbuf.terrainFs = res.fshader("terrain_minor_main_material");

    gbuf.lightDirection.setName("lightDirection");
    gbuf.lightColor    .setName("lightColor");
    gbuf.lightAblimient.setName("lightAblimient");
    gbuf.view          .setName("view");

    smap.vs = vsHolder.load("./data/sh/shadow_map.vert");
    smap.fs = fsHolder.load("./data/sh/shadow_map.frag");

    transparentData.vs = res.vshader("main_material");
    transparentData.fs = res.fshader("main_material");

    transparentData.vsAdd = res.vshader("blend_add");
    transparentData.fsAdd = res.fshader("blend_add");

    displaceData.vs = res.vshader("displace");
    displaceData.fs = res.fshader("displace");

    displaceData.vsWater = res.vshader("water");
    displaceData.fsWater = res.fshader("water");

    gaussData.texture.setName("texture");
    gaussData.vs = vsHolder.load("./data/sh/postProcess.vert");
    gaussData.fs = fsHolder.load("./data/sh/gauss.frag");

    bloomData.vs = gaussData.vs;
    bloomData.brightPass = fsHolder.load("./data/sh/brightPass.frag");
    bloomData.combine    = fsHolder.load("./data/sh/bloomCombine.frag");
    bloomData.b[0].setName("b0");
    bloomData.b[1].setName("b1");
    bloomData.b[2].setName("b2");

    water.waterHeightMap = texHolder.load("./data/textures/water/hmap.png");
    water.vs = vsHolder.load("./data/sh/htonorm.vert");
    water.fs = fsHolder.load("./data/sh/htonorm.frag");

    finalData.vs = gaussData.vs;
    finalData.fs     = fsHolder.load("./data/sh/final.frag");
    finalData.avatar = fsHolder.load("./data/sh/avatar_final.frag");

    finalData.scene.setName("scene");
    finalData.bloom.setName("bloom");
    finalData.glow .setName("glow");

    omniData.vs = res.vshader("omni");
    omniData.fs = res.fshader("omni");

    scrOffset.setName("dTexCoord");
    scrOffset.set( 1.0f/screenSize.w, 1.0f/screenSize.h );

    cpyOffset = scrOffset;

    bltData.texture.setName("texture");
    bltData.vs = vsHolder.load("./data/sh/blitShader.vert");
    bltData.fs = fsHolder.load("./data/sh/blitShader.frag");

    ssaoData.vs     = gaussData.vs;
    ssaoData.fs     = fsHolder.load("./data/sh/ssao_macro.frag");
    ssaoData.detail = fsHolder.load("./data/sh/ssao_detail.frag");
    ssaoData.accept = fsHolder.load("./data/sh/ssao_accept.frag");

    ssaoData.texture.setName("texture");
    ssaoData.blured. setName("blured");
    ssaoData.macro.  setName("macro");
    ssaoData.scene  .setName("scene");
    ssaoData.diff   .setName("diff");
    ssaoData.ssao.   setName("ssao");
    ssaoData.lightAblimient.setName("lightAblimient");

    reflect.uClamp = MyGL::Texture2d::ClampMode::MirroredRepeat;
    reflect.vClamp = reflect.uClamp;

    bufSampler.uClamp = MyGL::Texture2d::ClampMode::ClampToBorder;
    bufSampler.vClamp = bufSampler.uClamp;
    }

MyGL::Matrix4x4 GraphicsSystem::makeShadowMatrix( const MyGL::Scene & scene,
                                                  double * dir ){
    MyGL::Matrix4x4 mat;

    float dist = 0.4, x = 2, y = 2, s = 0.3, cs = 0.3;

    const MyGL::Camera &view =
        reinterpret_cast<const MyGL::Camera&>( scene.camera() );

    x = view.x();
    y = view.y();
    s /= std::max( view.distance(), 1.0 )/3.0;

    double l = sqrt( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );

    for( int i=0; i<3; ++i )
      dir[i] = (dir[i]/l);

    MyGL::Float m[4*4] = {
       s, 0,  0, 0,
       0, s,  0, 0,
       s*dir[0], s*dir[1], cs*dir[2], 0,
       -s*x, -s*y, dist, 1,
       };

    mat.setData( m );

    return mat;
    }

void GraphicsSystem::resizeEvent( int w, int h, bool isFullScreen ){
  screenSize.w = w;
  screenSize.h = h;

  device.reset( makeOpt(isFullScreen) );
  }

MyGL::Device::Options GraphicsSystem::makeOpt(bool isFullScreen) {
  MyGL::Device::Options opt;
  opt.windowed = !isFullScreen;
  opt.vSync    = !opt.windowed;

  return opt;
  }

void GraphicsSystem::load( Resource &res, MainGui &gui, int w, int h ) {
  widget = &gui;
  //waterHeightMap = &res.texture("water/height");
  makeRenderAlgo( res, gui, w, h );
  }

MyGL::Texture2d GraphicsSystem::depth(int w, int h) {
  return localTex.create(w,h, MyGL::AbstractTexture::Format::Depth24 );
  }

MyGL::Texture2d GraphicsSystem::depth( const MyGL::Size & sz ) {
  return localTex.create( sz.w, sz.h, MyGL::AbstractTexture::Format::Depth24 );
  }

bool GraphicsSystem::render( const MyGL::Scene &scene,
                             ParticleSystemEngine & particles,
                             size_t dt) {
  if( !device.startRender() )
    return false;

  time = dt;//(time+dt);
  unsigned tx = time%(16*1024);

  BlushMaterial::wind = sin( 2.0*M_PI*tx/(16*1024.0) );

  particles.exec();

  MyGL::Texture2d gbuffer[4];
  MyGL::Texture2d mainDepth = depth( screenSize );

  for( int i=0; i<3; ++i ){
    gbuffer[i] = localTex.create( screenSize.w, screenSize.h,
                                  MyGL::Texture2d::Format::RGBA );
    }
  gbuffer[3] = localTex.create( screenSize.w, screenSize.h,
                                MyGL::Texture2d::Format::RG16 );

  renderScene( scene, gbuffer, mainDepth,
               1024, false );

  if( widget )
    gui.exec( *widget, gbuffer[0], mainDepth, device );

  MyGL::Texture2d glow, bloomTex;
  drawGlow( glow, mainDepth, scene, 512 );

  bloom( bloomTex, gbuffer[0] );
  //blt( glow );
  //blt( bloomTex );

  MyGL::Texture2d final = localTex.create( screenSize.w, screenSize.h );
  { final.setSampler( reflect );

    MyGL::Texture2d depth = this->depth( final.width(),
                                         final.height() );

    MyGL::Render render( device,
                         final, depth,
                         bloomData.vs, bloomData.brightPass );

    render.setRenderState( MyGL::RenderState::PostProcess );

    cpyOffset.set( 1.0/final.width(), 1.0/final.height() );
    device.setUniform( bloomData.vs, cpyOffset );

    finalData.bloom.set( &bloomTex );
    finalData.scene.set( &gbuffer[0] );
    finalData.glow .set( &glow );

    device.setUniform( finalData.fs, finalData.bloom );
    device.setUniform( finalData.fs, finalData.scene );
    device.setUniform( finalData.fs, finalData.glow );

    ppHelper.drawFullScreenQuad( device, finalData.vs, finalData.fs );
    }

  //blt( gbuffer[0] );
  //blt( mainDepth );
  //blt( waterWaves );
//  blt( shadowMap );
  blt( final );
  //blt( gao );
  //blt( ssaoTexDet );
  //blt( gbuffer[2] );

  device.present();
  return 1;
  }

void GraphicsSystem::fillShadowMap( MyGL::Texture2d& shadowMap,
                                    const MyGL::Scene & scene ) {
  MyGL::Texture2d depthSm = depth( shadowMap.width(), shadowMap.height() );

  fillShadowMap( shadowMap, depthSm, scene,
                 scene.objects<MyGL::ShadowMapPassBase::Material>(), 1 );
  fillShadowMap( shadowMap, depthSm, scene,
                 scene.objects<BlushShMaterial>(), 0 );
  }

void GraphicsSystem::fillShadowMap( MyGL::Texture2d& sm,
                                    MyGL::Texture2d& depthSm,
                                    const MyGL::Scene & scene,
                                    const MyGL::Scene::Objects &v,
                                    bool clr ) {
  MyGL::RenderState rstate;
  rstate.setCullFaceMode( MyGL::RenderState::CullMode::front );

  MyGL::Render render( device,
                       sm, depthSm,
                       smap.vs, smap.fs );
  if( clr )
    render.clear( MyGL::Color(1.0), 1 );

  render.setRenderState( rstate );

  const MyGL::AbstractCamera & camera = scene.camera();
  MyGL::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };
  MyGL::Matrix4x4 matrix = makeShadowMatrix(scene, dir);

  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::AbstractGraphicObject& ptr = v[i].object();

    MyGL::Matrix4x4 m = matrix;

    if( clr )
      m.mul( ptr.transform() ); else
      m.mul( BlushMaterial::animateObjMatrix( ptr.transform() ) );

    if( scene.viewTester().isVisible( ptr, m ) ){
      device.setUniform( smap.vs, m, "mvpMatrix" );
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }

  }

void GraphicsSystem::setupLight( const MyGL::Scene & scene,
                                 MyGL::FragmentShader & fs,
                                 const MyGL::Texture2d & sm ){
  if( scene.lights().direction().size()>0 ){
    MyGL::DirectionLight l = scene.lights().direction()[0];

    gbuf.lightDirection.set( l, MyGL::Direction );
    device.setUniform( fs, gbuf.lightDirection );

    gbuf.lightColor.set( l, MyGL::LightColor );
    device.setUniform( fs, gbuf.lightColor );

    gbuf.lightAblimient.set( l, MyGL::LightAblimient );
    device.setUniform( fs, gbuf.lightAblimient );

    MyGL::Matrix4x4 vm = scene.camera().view();

    float view[3] = { float(vm.at(2,0)),
                      float(vm.at(2,1)),
                      float(vm.at(2,2)) };
    float len = sqrt(view[0]*view[0] + view[1]*view[1] + view[2]*view[2]);

    for( int i=0; i<3; ++i )
      view[i] /= len;

    gbuf.view.set( view );
    device.setUniform( fs, gbuf.view );
    device.setUniform( fs, sm, "shadowMap" );
    }
  }

void GraphicsSystem::fillGBuf( MyGL::Texture2d* gbuffer,
                               MyGL::Texture2d& mainDepth,
                               const MyGL::Texture2d& sm,
                               const MyGL::Scene & scene ) {
  setupLight( scene, gbuf.terrainFs, sm );

  drawObjects( gbuf.terrainVs,
               gbuf.terrainFs,
               gbuffer, mainDepth,
               scene, scene.objects<TerrainZPass>(), true );
  {
    MyGL::Texture2d d = depth( mainDepth.width(), mainDepth.height() );
    MyGL::Render render( device,
                         gbuffer, 4,
                         d,
                         gbuf.terrainVs, gbuf.terrainFs );
    render.clear( MyGL::Color(0) );
  }

  drawObjects( gbuf.terrainVs,
               gbuf.terrainFs,
               gbuffer, mainDepth,
               scene, scene.objects<TerrainMinorMaterial>(), false );

  setupLight( scene, gbuf.fs, sm );
  drawObjects( gbuffer, mainDepth,
               scene, scene.objects<MainMaterial>(), false );

  drawObjects( gbuffer, mainDepth,
               scene, scene.objects<BlushMaterial>(), false );

  drawObjects( transparentData.vsAdd, transparentData.fsAdd,
               gbuffer, mainDepth,
               scene, scene.objects<AddMaterial>() );

  drawObjects( transparentData.vsAdd, transparentData.fsAdd,
               gbuffer, mainDepth,
               scene, scene.objects<TransparentMaterialZPass>() );

  setupLight( scene, transparentData.fs, sm );
  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer, mainDepth,
               scene, scene.objects<TransparentMaterial>() );

  }

void GraphicsSystem::drawOmni( MyGL::Texture2d *gbuffer,
                               MyGL::Texture2d &mainDepth,
                               const MyGL::Scene &scene ) {
  MyGL::Render render( device,
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

  float tc[] = { 1.0f/gbuffer[3].width(), 1.0f/gbuffer[3].height() };
  device.setUniform( omniData.fs, tc, 2, "dTexCoord");

  const MyGL::AbstractCamera & camera = scene.camera();
  const MyGL::Scene::Objects &v = scene.objects<OmniMaterial>();
  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::GraphicObject& ptr = (const MyGL::GraphicObject&)v[i].object();

    float pos[3] = { };//ptr.x(), ptr.y(), ptr.z() };
    device.setUniform( omniData.vs, pos, 3, "cenPos");

    MyGL::Matrix4x4 mat = camera.projective();
    mat.mul( camera.view() );
    mat.mul( ptr.transform() );
    mat.inverse();

    device.setUniform( omniData.fs, mat, "invMatrix");

    if( scene.viewTester().isVisible( ptr, camera ) ){
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }
  }

void GraphicsSystem::drawObjects( MyGL::Texture2d* gbuffer,
                                  MyGL::Texture2d& mainDepth,
                                  const MyGL::Scene &scene,
                                  const MyGL::Scene::Objects &v,
                                  bool clr ) {
  drawObjects(gbuf.vs, gbuf.fs, gbuffer, mainDepth, scene, v, clr);
  }

void GraphicsSystem::drawObjects( MyGL::VertexShader   vs,
                                  MyGL::FragmentShader fs,

                                  MyGL::Texture2d* gbuffer,
                                  MyGL::Texture2d& mainDepth,
                                  const MyGL::Scene &scene,
                                  const MyGL::Scene::Objects &v,
                                  bool clr ) {
  MyGL::Render render( device,
                       gbuffer, 4,
                       mainDepth,
                       vs, fs );
  if( clr )
    render.clear( MyGL::Color(0.0), 1 );

  const MyGL::AbstractCamera & camera = scene.camera();

  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::AbstractGraphicObject& ptr = v[i].object();

    if( scene.viewTester().isVisible( ptr, camera ) ){
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }
  }

void GraphicsSystem::drawTranscurent( MyGL::Texture2d& screen,
                                      MyGL::Texture2d& mainDepth,
                                      MyGL::Texture2d& sceneCopy,
                                      const MyGL::Scene &scene,
                                      const MyGL::Scene::Objects &v ) {
  MyGL::Render render( device,
                       screen,
                       mainDepth,
                       displaceData.vs,
                       displaceData.fs );

  device.setUniform( displaceData.fs,
                     sceneCopy,
                     "scene" );
  float tc[] = { 1.0f/sceneCopy.width(), 1.0f/sceneCopy.height() };
  device.setUniform( displaceData.fs, tc, 2, "dTexCoord");

  const MyGL::AbstractCamera & camera = scene.camera();

  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::AbstractGraphicObject& ptr = v[i].object();

    if( scene.viewTester().isVisible( ptr, camera ) ){
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }
  }

void GraphicsSystem::drawWater( MyGL::Texture2d& screen,
                                MyGL::Texture2d& mainDepth,
                                MyGL::Texture2d& sceneCopy,
                                MyGL::Texture2d& sm,
                                MyGL::Texture2d& sceneDepth,
                                const MyGL::Scene &scene,
                                const MyGL::Scene::Objects &v ) {

  MyGL::Texture2d waterWaves;
  waves( waterWaves, water.waterHeightMap );

  setupLight( scene, displaceData.fsWater, sm );
  MyGL::Render render( device,
                       screen,
                       mainDepth,
                       displaceData.vsWater,
                       displaceData.fsWater );

  device.setUniform( displaceData.fsWater,
                     sceneCopy,
                     "scene" );
  float tc[] = { 1.0f/sceneCopy.width(), 1.0f/sceneCopy.height() };
  device.setUniform( displaceData.fsWater, tc, 2, "dTexCoord");
  device.setUniform( displaceData.fsWater, waterWaves, "normalMap");
  device.setUniform( displaceData.fsWater, sceneDepth, "sceneDepth");

  float dc[] = {0,0};
  device.setUniform( displaceData.fsWater, dc, 2, "dWaterCoord");

  const MyGL::AbstractCamera & camera = scene.camera();

  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::AbstractGraphicObject& ptr = v[i].object();

    MyGL::Matrix4x4 mat = camera.projective();
    mat.mul( camera.view() );
    mat.mul( ptr.transform() );
    mat.inverse();

    device.setUniform( displaceData.fsWater, mat, "invMatrix");

    if( scene.viewTester().isVisible( ptr, camera ) ){
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }
  }

void GraphicsSystem::drawGlow (MyGL::Texture2d &out,
                               MyGL::Texture2d &depth,
                               const MyGL::Scene &scene,
                               int size ) {
  MyGL::Texture2d buffer = localTex.create( depth.width(),
                                            depth.height(),
                                            MyGL::Texture2d::Format::RGBA );

  {
    MyGL::Render render( device,
                         buffer,
                         depth,
                         glowData.vs,
                         glowData.fs );
    render.clear( MyGL::Color(0,0,0,1) );

    const MyGL::AbstractCamera & camera = scene.camera();
    const MyGL::Scene::Objects & v = scene.objects<GlowMaterial>();

    for( size_t i=0; i<v.size(); ++i ){
      const MyGL::AbstractGraphicObject& ptr = v[i].object();

      if( scene.viewTester().isVisible( ptr, camera ) ){
        render.draw( v[i].material(), ptr,
                     ptr.transform(), camera );
        }
      }
    }

  //int size = 512;
  MyGL::Texture2d tmp;
  copy ( tmp, buffer,  size, size );
  gauss( buffer, tmp,  size, size, 1.0, 0.0 );
  gauss( out,  buffer, size, size, 0.0, 1.0 );

  //out = tmp;
  }

void GraphicsSystem::blt( const MyGL::Texture2d & tex ) {
  MyGL::Render render( device, bltData.vs, bltData.fs );

  render.setRenderState( MyGL::RenderState::PostProcess );

  bltData.texture.set( &tex );
  device.setUniform( bltData.vs, scrOffset );
  device.setUniform( bltData.fs, bltData.texture );

  ppHelper.drawFullScreenQuad( device, bltData.vs, bltData.fs );
  }

void GraphicsSystem::copy( MyGL::Texture2d &out,
                           const MyGL::Texture2d& in ) {
  copy(out, in, in.width(), in.height() );
  }

void GraphicsSystem::copy( MyGL::Texture2d &out,
                           const MyGL::Texture2d& in,
                           int w, int h ) {
  out = localTex.create( w,h );
  out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth( w,h );

  MyGL::Render render( device,
                       out, depth,
                       bltData.vs, bltData.fs );

  render.setRenderState( MyGL::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( bltData.vs, cpyOffset );
  device.setUniform( bltData.fs, bltData.texture );

  ppHelper.drawFullScreenQuad( device, bltData.vs, bltData.fs );
  }

void GraphicsSystem::copyDepth( MyGL::Texture2d &out,
                                const MyGL::Texture2d& in,
                                int w, int h ) {
  //int w = in.width(), h = in.height();

  out = localTex.create( w, h, MyGL::Texture2d::Format::RG16 );
  out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth( w,h );

  MyGL::Render render( device,
                       out, depth,
                       bltData.vs, bltData.fs );

  render.setRenderState( MyGL::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( bltData.vs, cpyOffset );
  device.setUniform( bltData.fs, bltData.texture );

  ppHelper.drawFullScreenQuad( device, bltData.vs, bltData.fs );
  }

void GraphicsSystem::gauss( MyGL::Texture2d &out,
                            const MyGL::Texture2d& in,
                            int w, int h,
                            float dx, float dy ) {
  out = localTex.create( w,h );
  out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth( w,h );

  MyGL::Render render( device,
                       out, depth,
                       gaussData.vs, gaussData.fs );

  render.setRenderState( MyGL::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( gaussData.vs, cpyOffset );

  cpyOffset.set( dx/w, dy/h );
  device.setUniform( gaussData.fs, cpyOffset );
  device.setUniform( gaussData.fs, bltData.texture );

  ppHelper.drawFullScreenQuad( device, gaussData.vs, gaussData.fs );
  }

void GraphicsSystem::bloom( MyGL::Texture2d &result,
                            const MyGL::Texture2d &in ) {
  //bloomData
  const int w = 256, h = w;

  result = localTex.create( w,h );
  MyGL::Texture2d tmp[4];

  MyGL::Size sizes[3] = {
    {256, 256}, {64, 64}, {16, 16}
    };

  {
    tmp[0] = localTex.create( w,h );
    MyGL::Texture2d depth = this->depth( w,h );

    MyGL::Render render( device,
                         tmp[0], depth,
                         bloomData.vs, bloomData.brightPass );

    render.setRenderState( MyGL::RenderState::PostProcess );

    bltData.texture.set( &in );
    cpyOffset.set( 1.0f/w, 1.0f/h );
    device.setUniform( bloomData.vs, cpyOffset );

    device.setUniform( bloomData.brightPass, bltData.texture );

    ppHelper.drawFullScreenQuad( device, bloomData.vs, bloomData.brightPass );
    }

  for( int i=1; i<4; ++i ){
    MyGL::Texture2d & out = tmp[i];

    const MyGL::Size & sz = sizes[i-1];
    out = localTex.create( sz.w, sz.h );
    out.setSampler( reflect );

    MyGL::Texture2d htmp;

    gauss( htmp, tmp[i-1], sz.w, sz.h, 1.0, 0.0 );
    gauss(  out,     htmp, sz.w, sz.h, 0.0, 1.0 );
    }

  { result.setSampler( reflect );

    MyGL::Texture2d depth = this->depth( result.width(),
                                         result.height() );

    MyGL::Render render( device,
                         result, depth,
                         bloomData.vs, bloomData.brightPass );

    render.setRenderState( MyGL::RenderState::PostProcess );

    cpyOffset.set( 0,0 );
    device.setUniform( bloomData.vs, cpyOffset );

    for( int i=0; i<3; ++i ){
      bloomData.b[i].set( &tmp[i+1] );
      device.setUniform( bloomData.combine, bloomData.b[i] );
      }

    ppHelper.drawFullScreenQuad( device, bloomData.vs, bloomData.combine );
    }
  }

void GraphicsSystem::waves( MyGL::Texture2d &out,
                            const MyGL::Texture2d& in) {
  int w = in.width(), h = in.height();

  out = localTex.create( w,h );
  out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth(w,h);
  MyGL::Render render( device,
                       out, depth,
                       water.vs, water.fs );

  render.setRenderState( MyGL::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( water.vs, cpyOffset );

  device.setUniform( water.fs, bltData.texture );

  //sin( 2.0*M_PI*time/(4*1024.0) )
  //static unsigned time = 0;
  //++time;
  float t = 2*M_PI*float(time%2048)/2048.0f;

  device.setUniform ( water.fs, &t, 1, "time" );

  ppHelper.drawFullScreenQuad( device, water.vs, water.fs );
  }

void GraphicsSystem::ssaoDetail( MyGL::Texture2d &out,
                                 const MyGL::Texture2d &in ,
                                 const MyGL::Texture2d &macro ) {
  int w = 512, h = w;

  MyGL::Texture2d tmp;
  copy( tmp, in, w, h );
  gauss( out, tmp, w, h, 2, 0 );
  gauss( tmp, out, w, h, 0, 2 );

  MyGL::Texture2d depth = this->depth( w,h );

  MyGL::Render render( device,
                       out, depth,
                       ssaoData.vs, ssaoData.detail );

  render.setRenderState( MyGL::RenderState::PostProcess );

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

void GraphicsSystem::ssao( MyGL::Texture2d &out,
                           const MyGL::Texture2d &in,
                           const MyGL::Scene & scene ) {
  int w = in.width(), h = in.height();

  MyGL::Texture2d gao;
  ssaoGMap( scene, gao );
  const MyGL::AbstractCamera &camera = scene.camera();

  MyGL::Matrix4x4 mat = camera.projective();
  mat.mul( camera.view() );
  mat.inverse();

  double dir[] = {0,0, -1};
  MyGL::Matrix4x4 shMatrix = makeShadowMatrix(scene, dir);

  const MyGL::Camera &view =
      reinterpret_cast<const MyGL::Camera&>( scene.camera() );
  float scaleSize = 0.3/std::max( view.distance(), 1.0 )/3.0;

  out = localTex.create( w, h, MyGL::Texture2d::Format::Luminance16 );
  out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth( w,h );

  MyGL::Render render( device,
                       out, depth,
                       ssaoData.vs, ssaoData.fs );

  render.setRenderState( MyGL::RenderState::PostProcess );

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

void GraphicsSystem::aceptSsao( const MyGL::Scene & s,
                                MyGL::Texture2d &out,
                                const MyGL::Texture2d &scene,
                                const MyGL::Texture2d &diff,
                                const MyGL::Texture2d &ssao ) {
  //ssaoData.lightAblimient.set(1,1,1);

  if( s.lights().direction().size()>0 ){
    MyGL::DirectionLight l = s.lights().direction()[0];
    ssaoData.lightAblimient.set( l, MyGL::LightAblimient );
    }

  int w = scene.width(), h = scene.height();

  out = localTex.create( w,h );
  out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth( w,h );

  MyGL::Render render( device,
                       out, depth,
                       ssaoData.vs, ssaoData.accept );

  render.setRenderState( MyGL::RenderState::PostProcess );

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

void GraphicsSystem::ssaoGMap( const MyGL::Scene &scene,
                               MyGL::Texture2d &sm  ) {
  sm = localTex.create( 128, 128,
                        MyGL::AbstractTexture::Format::Luminance16 );

  const MyGL::Scene::Objects &v = scene.objects<MyGL::ShadowMapPassBase::Material>();

  MyGL::RenderState rstate;
  rstate.setCullFaceMode( MyGL::RenderState::CullMode::front );

  MyGL::Texture2d depthSm = depth( sm.width(), sm.height() );
  MyGL::Render render( device,
                       sm, depthSm,
                       smap.vs, smap.fs );
  render.clear( MyGL::Color(1.0), 1 );
  render.setRenderState( rstate );

  const MyGL::AbstractCamera & camera = scene.camera();

  double dir[] = {0,0, -1};
  MyGL::Matrix4x4 matrix = makeShadowMatrix(scene, dir);

  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::AbstractGraphicObject& ptr = v[i].object();
    MyGL::Matrix4x4 m = matrix;
    m.mul( ptr.transform() );

    if( scene.viewTester().isVisible( ptr, m ) ){
      device.setUniform( smap.vs, m, "mvpMatrix" );
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }

  }

void GraphicsSystem::renderScene( const MyGL::Scene &scene,
                                  MyGL::Texture2d gbuffer[4],
                                  MyGL::Texture2d &mainDepth,
                                  int shadowMapSize,
                                  bool useAO ) {
  MyGL::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };

  closure.shadow.matrix = makeShadowMatrix( scene, dir );

  MyGL::Texture2d shadowMap = localTex.create( shadowMapSize,
                                               shadowMapSize,
                                               MyGL::AbstractTexture::Format::Luminance16 );
  fillShadowMap( shadowMap, scene );

  fillGBuf( gbuffer, mainDepth, shadowMap, scene );

  drawOmni( gbuffer, mainDepth, scene );

  if( useAO ){
    MyGL::Texture2d ssaoTex;
    ssao( ssaoTex, gbuffer[3], scene );
    //ssaoDetail( ssaoTexDet, gbuffer[2], ssaoTex );

    MyGL::Texture2d aoAcepted;
    aceptSsao( scene, aoAcepted, gbuffer[0], gbuffer[1], ssaoTex );
    gbuffer[0] = aoAcepted;
    }

  //blt( shadowMap );
  MyGL::Texture2d sceneCopy;
  copy( sceneCopy, gbuffer[0] );

  drawTranscurent( gbuffer[0], mainDepth, sceneCopy,
                   scene,
                   scene.objects<DisplaceMaterial>());

  drawWater( gbuffer[0], mainDepth,
             sceneCopy, shadowMap, gbuffer[3],
             scene,
             scene.objects<WaterMaterial>());

  if(1){
    setupLight( scene, transparentData.fs, shadowMap );
    drawObjects( transparentData.vs, transparentData.fs,
                 gbuffer, mainDepth,
                 scene, scene.objects<TransparentMaterialNoZW>() );
    }
  }

void GraphicsSystem::renderSubScene( const MyGL::Scene &scene,
                                     MyGL::Texture2d &out  ) {
  int w = out.width(),
      h = out.height();

  MyGL::Texture2d gbuffer[4];
  MyGL::Texture2d mainDepth = this->depth(w,h);

  for( int i=0; i<3; ++i ){
    gbuffer[i] = localTex.create( w, h,
                                  MyGL::Texture2d::Format::RGBA );
    }
  gbuffer[3] = localTex.create( w, h,
                                MyGL::Texture2d::Format::RG16 );

  renderScene( scene, gbuffer, mainDepth, 256, 0 );
  MyGL::Texture2d glow;
  drawGlow( glow, mainDepth, scene, 128 );

  { MyGL::Texture2d depth = this->depth( out.width(),
                                         out.height() );

    MyGL::Render render( device,
                         out, depth,
                         bloomData.vs, bloomData.brightPass );

    render.setRenderState( MyGL::RenderState::PostProcess );

    cpyOffset.set( 1.0/out.width(), 1.0/out.height() );
    device.setUniform( bloomData.vs, cpyOffset );

    finalData.scene.set( &gbuffer[0] );
    finalData.glow .set( &glow );

    //device.setUniform( finalData.fs, finalData.bloom );
    device.setUniform( finalData.avatar, finalData.scene );
    device.setUniform( finalData.avatar, finalData.glow );

    ppHelper.drawFullScreenQuad( device, finalData.vs, finalData.avatar );
    }

  //copy( out, glow );
  }
