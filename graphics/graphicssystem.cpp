#include "graphicssystem.h"

#include <MyGL/Model>
#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

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
#include "graphics/warfogmaterial.h"
#include "graphics/grassmaterial.h"

#include "graphics/particlesystemengine.h"

#include <resource.h>

GraphicsSystem::GraphicsSystem( void *hwnd, int w, int h,
                                bool isFullScreen,
                                int smSize = 1024 )
  : device(directx, makeOpt(isFullScreen), hwnd ),
    texHolder ( device ),
    localTex  ( device ),
    vboHolder ( device ),
    lvboHolder( device ),
    iboHolder ( device ),
    vsHolder  ( device ),
    fsHolder  ( device ),

    ppHelper( vboHolder, iboHolder ),
    gui( vsHolder.load("./data/sh/gui.vert"),
         fsHolder.load("./data/sh/gui.frag"),
         lvboHolder,
         screenSize  ) {
  widget    = 0;
  time      = -1;
  particles = 0;

  useFog    = false;

  lvboHolder.setReserveSize( 64*8092 );
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
    gaussData.vs = vsHolder.load("./data/sh/postProcess.vert");
    gaussData.fs = fsHolder.load("./data/sh/gauss.frag");

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

    bloomData.vs = gaussData.vs;
    bloomData.brightPass = fsHolder.load("./data/sh/brightPass.frag");
    bloomData.combine    = fsHolder.load("./data/sh/bloomCombine.frag");
    bloomData.b[0].setName("b0");
    bloomData.b[1].setName("b1");
    bloomData.b[2].setName("b2");

    water.waterHeightMap[0] = texHolder.load("./data/textures/water/hmap.png");
    water.waterHeightMap[1] = texHolder.load("./data/textures/water/hmap1.png");

    water.envMap = res.texture("sky0/diff");

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

    reflect.uClamp = MyGL::Texture2d::ClampMode::MirroredRepeat;
    reflect.vClamp = reflect.uClamp;

    bufSampler.uClamp = MyGL::Texture2d::ClampMode::ClampToBorder;
    bufSampler.vClamp = bufSampler.uClamp;
    }

float GraphicsSystem::smMatSize( const Scene &scene ) {
    float s = 0.3;

    const MyGL::Camera &view =
        reinterpret_cast<const MyGL::Camera&>( scene.camera() );

    s /= std::max( view.distance(), 1.0 )/3.0;
    s = std::min(s, 0.4f);

    return s;
    }

MyGL::Matrix4x4 GraphicsSystem::makeShadowMatrix( const Scene & scene ){
  MyGL::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };

  return makeShadowMatrix(scene,dir);
  }

MyGL::Matrix4x4 GraphicsSystem::makeShadowMatrix( const Scene & scene,
                                                  double * dir ){
    MyGL::Matrix4x4 mat;

    float dist = 0.4, x = 2, y = 2, z = 0, s = 0.3, cs = 0.3;

    const MyGL::Camera &view =
        reinterpret_cast<const MyGL::Camera&>( scene.camera() );

    x = view.x();
    y = view.y();
    z = view.z();

    s = smMatSize(scene);

    double l = sqrt( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );

    for( int i=0; i<3; ++i )
      dir[i] = (dir[i]/l);

    MyGL::Float m[4*4] = {
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

bool GraphicsSystem::render( Scene &scene,
                             ParticleSystemEngine & par,
                             MyGL::Camera   camera,
                             size_t dt) {
  if( !device.startRender() )
    return false;

  //return false;
  onRender( std::max<size_t>(dt-time, 0) );

  time = dt;//(time+dt);
  unsigned tx = time%(16*1024);

  Material::wind = sin( 2.0*M_PI*tx/(16*1024.0) );

  particles = &par;

  MyGL::Texture2d gbuffer[4], rsm[4];
  MyGL::Texture2d mainDepth = depth( screenSize );

  for( int i=0; i<3; ++i ){
    gbuffer[i] = localTex.create( screenSize.w, screenSize.h,
                                  MyGL::Texture2d::Format::RGBA );
    }
  gbuffer[3] = localTex.create( screenSize.w, screenSize.h,
                                MyGL::Texture2d::Format::RG16 );

  //buildRSM( scene, rsm, 512, 0 );
  scene.setCamera( camera );
  /*
  blt( rsm[0] );
  device.present();
  return 1;
  */

  renderScene( scene,
               scene.camera(),
               gbuffer, mainDepth, rsm,
               2048, true );

  MyGL::Texture2d fog;
  drawFogOfWar(fog, scene);
  aceptFog( gbuffer[0], fog );  

  if( widget )
    gui.exec( *widget, gbuffer[0], mainDepth, device );

  MyGL::Texture2d glow, bloomTex;
  drawGlow( glow, mainDepth, scene, 512 );
  aceptFog( glow, fog );

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
  //blt( gbuffer[2] );

  device.present();
  return 1;
  }

void GraphicsSystem::fillShadowMap( MyGL::Texture2d& shadowMap,
                                    const Scene & scene ) {
  MyGL::Texture2d depthSm = depth( shadowMap.width(), shadowMap.height() );

  fillShadowMap( shadowMap, depthSm, scene,
                 scene.shadowCasters() );
  }

void GraphicsSystem::fillShadowMap( MyGL::Texture2d& sm,
                                    MyGL::Texture2d& depthSm,
                                    const Scene & scene,
                                    const Scene::Objects &v ) {
  MyGL::RenderState rstate;
  rstate.setCullFaceMode( MyGL::RenderState::CullMode::front );

  MyGL::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };
  MyGL::Matrix4x4 matrix = makeShadowMatrix(scene, dir);

  MyGL::Render render( device,
                       sm, depthSm,
                       smap.vs, smap.fs );
  {
    render.clear( MyGL::Color(1.0), 1 );
    if( !(sm.width()==1 && sm.height()==1) ){
      MyGL::Matrix4x4 proj;
      proj.identity();

      particles->exec( matrix, proj, 0, true );
      }
    }

  if( sm.width()==1 && sm.height()==1 )
    return;

  render.setRenderState( rstate );
  //return;

  const MyGL::AbstractCamera & camera = scene.camera();

  for( size_t i=0; i<v.size(); ++i ){
    const AbstractGraphicObject& ptr = *v[i];
    MyGL::Matrix4x4 m = matrix;

    if( !ptr.material().usage.blush )
      m.mul( ptr.transform() ); else
      m.mul( Material::animateObjMatrix( ptr.transform() ) );

    if( scene.viewTester().isVisible( ptr, m ) ){
      MyGL::UniformTable table( device, smap.vs, smap.fs );
      MyGL::RenderState rs;

      table.add( m, "mvpMatrix", MyGL::UniformTable::Vertex );
      ptr.material().shadow( rs, ptr.transform(), camera, table );

      device.setRenderState(rs);
      render.draw( ptr );
      }
    }
  }

void GraphicsSystem::fillTranscurentMap( MyGL::Texture2d &sm,
                                         MyGL::Texture2d &depthSm,
                                         const Scene &scene) {

  MyGL::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  MyGL::Texture2d lightColor = localTex.create(1,1);
  {
    MyGL::Texture2d depthSm = depth(1,1);
    MyGL::Render render( device,
                         sm, depthSm,
                         transparentData.vsSh,
                         transparentData.fsSh );
    render.clear( light.color(), 1 );
    }

  MyGL::RenderState rstate;
  rstate.setCullFaceMode( MyGL::RenderState::CullMode::front );

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };
  MyGL::Render render( device,
                       sm, depthSm,
                       transparentData.vsSh,
                       transparentData.fsSh );
  render.clear( light.color(), 1 );

  if( sm.width()==1 && sm.height()==1 )
    return;

  setupLight( scene, transparentData.fsSh, sm, lightColor );

  render.setRenderState( rstate );
  RSMCamera c;
  c.v = makeShadowMatrix( scene, dir );
  c.p.setData( 1, 0, 0, 0,
               0,-1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1 );

  /*
  { auto v = scene.objects<TransparentMaterialShadow>();
    for( size_t i=0; i<v.size(); ++i ){
      const MyGL::AbstractGraphicObject& ptr = v[i].object();

      MyGL::Matrix4x4 m = c.v;
      m.mul( ptr.transform() );

      if( scene.viewTester().isVisible( ptr, m ) ){
        device.setUniform( transparentData.vsSh, m, "mvpMatrix" );
        render.draw( v[i].material(), ptr,
                     ptr.transform(), c );
        }
      }
    }*/
  }

void GraphicsSystem::setupLight( const Scene & scene,
                                 MyGL::FragmentShader & fs,
                                 const MyGL::Texture2d & sm,
                                 const MyGL::Texture2d & smCl ){
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

    device.setUniform( fs,     sm, "shadowMap"    );
    device.setUniform( fs,   smCl, "shadowMapCl"  );
    }
  }

void GraphicsSystem::fillGBuf( MyGL::Texture2d* gbuffer,
                               MyGL::Texture2d& mainDepth,
                               const MyGL::Texture2d& sm,
                               const MyGL::Texture2d& smCl,
                               const Scene & scene,
                               const MyGL::AbstractCamera & camera ) {
  setupLight( scene, gbuf.terrainFs, sm, smCl );
  particles->exec( scene.camera().view(),
                   scene.camera().projective(),
                   1 );

  setupLight( scene, gbuf.fs, sm, smCl );

  drawObjects( gbuf.terrainVs,
               gbuf.terrainFs,
               gbuffer,
               mainDepth,
               scene,
               camera,
               scene.terrainMinorObjects(),
               &Material::terrainMinorZ,
               true );
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
               gbuffer,
               mainDepth,
               scene,
               camera,
               scene.terrainMinorObjects(),
               &Material::terrainMinor,
               false );

  setupLight( scene, gbuf.fs, sm, smCl );

  drawObjects( gbuffer,
               mainDepth,
               scene,
               camera,
               scene.mainObjects(),
               &Material::gbuffer,
               false );

  /*

  setupLight( scene, gbuf.grassFs, sm, smCl );
  drawObjects( gbuf.grassVs, gbuf.grassFs,
               gbuffer, mainDepth,
               scene,
               camera,
               scene.objects<GrassMaterial>() );
               */

  drawObjects( transparentData.vsAdd, transparentData.fsAdd,
               gbuffer, mainDepth,
               scene,
               camera,
               scene.additiveObjects(),
               &Material::additive,
               false );

  setupLight( scene, transparentData.fs, sm, smCl );
  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer, mainDepth,
               scene,
               camera,
               scene.transparentObjects(),
               &Material::transparentZ,
               false );

  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer, mainDepth,
               scene,
               camera,
               scene.transparentObjects(),
               &Material::transparent,
               false );
  }

void GraphicsSystem::renderVolumeLight( const Scene &scene,
                                        MyGL::Texture2d &gbuffer,
                                        MyGL::Texture2d &mainDepth,
                                        MyGL::Texture2d &shadowMap ) {
  int w = 512,//gbuffer.width(),
      h = 512;//gbuffer.height();

  MyGL::Texture2d d = depth( w, h );
  MyGL::Texture2d vlTex = localTex.create(w, h,
                                          MyGL::AbstractTexture::Format::RGBA );

  { MyGL::Render render( device,
                         vlTex,
                         d,
                         volumetricData.vs,
                         volumetricData.fs );
    render.setRenderState( MyGL::RenderState::PostProcess );

    MyGL::Matrix4x4 mat = scene.camera().projective();
    mat.mul( scene.camera().view() );
    mat.inverse();

    MyGL::Matrix4x4 smM = closure.shadow.matrix;

    device.setUniform( volumetricData.fs, mat, "invMatrix");
    device.setUniform( volumetricData.fs, smM,  "shMatrix");
    device.setUniform( volumetricData.fs, shadowMap, "shadowMap");
    device.setUniform( volumetricData.fs, mainDepth, "mainDepth");

    cpyOffset.set( 1.0f/vlTex.width(), 1.0f/vlTex.height() );
    device.setUniform( volumetricData.vs, cpyOffset );

    ppHelper.drawFullScreenQuad( device, volumetricData.vs, volumetricData.fs );
    }

  { MyGL::Texture2d depth = this->depth( mainDepth.width(),
                                         mainDepth.height() );

    MyGL::Render render( device,
                         gbuffer, depth,
                         bltData.vs, bltData.fs );

    MyGL::RenderState rs = MyGL::RenderState::PostProcess;
    rs.setBlend(1);
    /*
    rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::dst_color,
                     MyGL::RenderState::AlphaBlendMode::zero );*/
    rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                     MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );

    render.setRenderState( rs );

    bltData.texture.set( &vlTex );
    cpyOffset.set( 1.0f/gbuffer.width(), 1.0f/gbuffer.height() );
    device.setUniform( bltData.vs, cpyOffset );
    device.setUniform( bltData.fs, bltData.texture );

    ppHelper.drawFullScreenQuad( device, bltData.vs, bltData.fs );
    }
  }

void GraphicsSystem::drawOmni( MyGL::Texture2d *gbuffer,
                               MyGL::Texture2d &mainDepth,
                               MyGL::Texture2d & sm,
                               const Scene &scene ) {
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

  device.setUniform( omniData.fs,
                     sm,
                     "shadowMap" );

  double dir[3] = {0,0,-1};
  MyGL::Matrix4x4 shM = makeShadowMatrix(scene,dir);


  float tc[] = { 1.0f/gbuffer[3].width(), 1.0f/gbuffer[3].height() };
  device.setUniform( omniData.fs, tc, 2, "dTexCoord");

  const MyGL::AbstractCamera & camera = scene.camera();
  const Scene::Objects &v = scene.omni();

  for( size_t i=0; i<v.size(); ++i ){
    const AbstractGraphicObject& ptr = *v[i];

    MyGL::Matrix4x4 mat = camera.projective();
    mat.mul( camera.view() );
    mat.mul( ptr.transform() );
    mat.inverse();

    MyGL::Matrix4x4 smat = shM;
    smat.mul( ptr.transform() );

    if( scene.viewTester().isVisible( ptr, camera ) ){
      MyGL::UniformTable table( render );
      MyGL::RenderState rs;

      ptr.material().omni( rs, ptr.transform(), camera, table, smat, mat );
      device.setRenderState(rs);
      render.draw( ptr );
      }
    }
  }

void GraphicsSystem::drawObjects( MyGL::Texture2d* gbuffer,
                                  MyGL::Texture2d& mainDepth,
                                  const Scene &scene,
                                  const MyGL::AbstractCamera & camera,
                                  const Scene::Objects &v,
                                  void (Material::*func)( MyGL::RenderState& /*d*/,
                                                          const MyGL::Matrix4x4 & /*object*/,
                                                          const MyGL::AbstractCamera&,
                                                          MyGL::UniformTable &,
                                                          const MyGL::Matrix4x4 & ) const,
                                  bool clr ) {
  drawObjects(gbuf.vs, gbuf.fs, gbuffer, mainDepth, scene, camera, v, func, clr);
  }

void GraphicsSystem::drawObjects( MyGL::VertexShader   & vs,
                                  MyGL::FragmentShader & fs,
                                  MyGL::Texture2d* gbuffer,
                                  MyGL::Texture2d& mainDepth,
                                  const Scene &scene,
                                  const MyGL::AbstractCamera& camera,
                                  const Scene::Objects &v,
                                  void (Material::*func)( MyGL::RenderState& /*d*/,
                                                          const MyGL::Matrix4x4 & /*object*/,
                                                          const MyGL::AbstractCamera&,
                                                          MyGL::UniformTable &,
                                                          const MyGL::Matrix4x4 & ) const,
                                  bool clr ) {
  MyGL::Render render( device,
                       gbuffer, 4,
                       mainDepth,
                       vs, fs );
  if( clr )
    render.clear( MyGL::Color(0.0), 1 );

  MyGL::Matrix4x4 matrix = makeShadowMatrix(scene);
  draw( render, scene, camera, v, func, matrix );
  }

void GraphicsSystem::drawTranscurent( MyGL::Texture2d& screen,
                                      MyGL::Texture2d& mainDepth,
                                      MyGL::Texture2d& sceneCopy,
                                      const Scene &scene,
                                      const Scene::Objects &v ) {
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

  MyGL::Matrix4x4 matrix = makeShadowMatrix(scene);
  draw( render, scene, camera, v, &Material::displace, matrix );
  }

template< class ... Args, class ... FArgs >
void GraphicsSystem::draw( MyGL::Render & render,
                           const Scene & scene,
                           const MyGL::AbstractCamera & camera,
                           const Scene::Objects & v,
                           void (Material::*func)( MyGL::RenderState& /*d*/,
                                                   const MyGL::Matrix4x4 & /*object*/,
                                                   const MyGL::AbstractCamera&,
                                                   MyGL::UniformTable &,
                                                   FArgs ... args ) const,
                           Args... args ){
  const MyGL::ViewTester & t = scene.viewTester();

  for( size_t i=0; i<v.size(); ++i ){
    const AbstractGraphicObject& ptr = *v[i];

    if( t.isVisible( ptr, camera ) ){
      MyGL::UniformTable table( render );
      MyGL::RenderState rs;

      (ptr.material().*func)( rs, ptr.transform(), camera, table, args... );
      device.setRenderState(rs);
      render.draw( ptr );
      }
    }
  }

void GraphicsSystem::drawWater( MyGL::Texture2d& screen,
                                MyGL::Texture2d& mainDepth,
                                MyGL::Texture2d& sceneCopy,
                                MyGL::Texture2d& sm,
                                MyGL::Texture2d& smCl,
                                MyGL::Texture2d& sceneDepth,
                                const Scene &scene,
                                const Scene::Objects &v ) {

  MyGL::Texture2d waterWaves;
  waves( waterWaves, water.waterHeightMap[0], water.waterHeightMap[1] );

  setupLight( scene, displaceData.fsWater, sm, smCl );
  MyGL::Render render( device,
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

  const MyGL::AbstractCamera & camera = scene.camera();
  MyGL::Matrix4x4 matrix = makeShadowMatrix(scene);

  for( size_t i=0; i<v.size(); ++i ){
    const AbstractGraphicObject& ptr = *v[i];

    MyGL::Matrix4x4 mat = camera.projective();
    mat.mul( camera.view() );
    mat.mul( ptr.transform() );
    mat.inverse();

    device.setUniform( displaceData.fsWater, mat, "invMatrix");

    if( scene.viewTester().isVisible( ptr, camera ) ){
      MyGL::UniformTable table( render );
      MyGL::RenderState rs;

      ptr.material().water( rs, ptr.transform(), camera, table, matrix );
      device.setRenderState(rs);
      render.draw( ptr );
      }
    }
  }

void GraphicsSystem::drawGlow (MyGL::Texture2d &out,
                               MyGL::Texture2d &depth,
                               const Scene &scene,
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

    const Scene::Objects & v = scene.glowObjects();

    for( size_t i=0; i<v.size(); ++i ){
      const AbstractGraphicObject& ptr = *v[i];

      if( scene.viewTester().isVisible( ptr, camera ) ){
        MyGL::UniformTable table( render );
        MyGL::RenderState rs;

        ptr.material().glowPass( rs, ptr.transform(), camera, table );
        device.setRenderState(rs);
        render.draw( ptr );
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

void GraphicsSystem::gauss_gb( MyGL::Texture2d &out,
                               const MyGL::Texture2d& in,
                               int w, int h,
                               float dx, float dy ) {
  out = localTex.create( w,h );
  out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth( w,h );

  MyGL::Render render( device,
                       out, depth,
                       gaussData.vsGB, gaussData.fsGB );

  render.setRenderState( MyGL::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( gaussData.vsGB, cpyOffset );

  cpyOffset.set( dx/w, dy/h );
  device.setUniform( gaussData.fsGB, cpyOffset );
  device.setUniform( gaussData.fsGB, bltData.texture );

  ppHelper.drawFullScreenQuad( device, gaussData.vsGB, gaussData.fsGB );
  }

void GraphicsSystem::gauss_b( MyGL::Texture2d &out,
                               const MyGL::Texture2d& in,
                               int w, int h,
                               float dx, float dy ) {
  out = localTex.create( w,h );
  out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth( w,h );

  MyGL::Render render( device,
                       out, depth,
                       gaussData.vsB, gaussData.fsB );

  render.setRenderState( MyGL::RenderState::PostProcess );

  bltData.texture.set( &in );
  cpyOffset.set( 1.0f/w, 1.0f/h );
  device.setUniform( gaussData.vsB, cpyOffset );

  cpyOffset.set( dx/w, dy/h );
  device.setUniform( gaussData.fsB, cpyOffset );
  device.setUniform( gaussData.fsB, bltData.texture );

  ppHelper.drawFullScreenQuad( device, gaussData.vsB, gaussData.fsB );
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
    tmp[0].setSampler( reflect );
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

void GraphicsSystem::drawFogOfWar( MyGL::Texture2d &out,
                                   const Scene & scene ) {
  int size = 256;

  MyGL::Texture2d depth  = this->depth( size, size );
  MyGL::Texture2d buffer = localTex.create( depth.width(),
                                            depth.height(),
                                            MyGL::Texture2d::Format::RGBA );

  {
    MyGL::Render render( device,
                         buffer,
                         depth,
                         fogOfWar.vs,
                         fogOfWar.fs );
    render.clear( MyGL::Color(0,0,0, 1), 1 );
    device.setUniform( fogOfWar.fs, fogView, "texture" );

    const MyGL::AbstractCamera & camera = scene.camera();

    const Scene::Objects & v = scene.fogOfWar();
    draw( render, scene, camera, v, &Material::fogOgWar, true );
    draw( render, scene, camera, v, &Material::fogOgWar, false );
    }

  MyGL::Texture2d tmp;
  copy ( tmp, buffer,  size, size );
  gauss( buffer, tmp,  size, size, 2.0, 0.0 );
  gauss( out,  buffer, size, size, 0.0, 2.0 );
  }

void GraphicsSystem::aceptFog( MyGL::Texture2d &in_out,
                               const MyGL::Texture2d &fog ) {
  if( !(fogView.width()>1 || fogView.height()>1 ) )
    return;

  MyGL::Texture2d depth = this->depth( in_out.width(),
                                       in_out.height() );
  MyGL::Texture2d tmp;
  copy(tmp, in_out);

  MyGL::Render render( device,
                       in_out, depth,
                       fogOfWar.vsAcept,
                       fogOfWar.fsAcept );

  render.setRenderState( MyGL::RenderState::PostProcess );

  cpyOffset.set( 1.0/tmp.width(), 1.0/tmp.height() );
  device.setUniform( fogOfWar.vsAcept, cpyOffset );

  device.setUniform( fogOfWar.fsAcept, tmp, "scene" );
  device.setUniform( fogOfWar.fsAcept, fog, "fog"   );

  ppHelper.drawFullScreenQuad( device, fogOfWar.vsAcept, fogOfWar.fsAcept );
  }

void GraphicsSystem::waves( MyGL::Texture2d &out,
                            const MyGL::Texture2d& in,
                            const MyGL::Texture2d& in1 ) {
  int w = 2*in.width(), h = 2*in.height();

  out = localTex.create( w,h );
  //out.setSampler( reflect );

  MyGL::Texture2d depth = this->depth(w,h);
  MyGL::Render render( device,
                       out, depth,
                       water.vs, water.fs );

  render.setRenderState( MyGL::RenderState::PostProcess );

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
                           const MyGL::Texture2d & gao,
                           const Scene & scene ) {
  int w = in.width(), h = in.height();

  const MyGL::AbstractCamera &camera = scene.camera();

  MyGL::Matrix4x4 mat = camera.projective();
  mat.mul( camera.view() );
  mat.inverse();

  double dir[] = {0,0, -1};
  MyGL::Matrix4x4 shMatrix = makeShadowMatrix(scene, dir);

  const MyGL::Camera &view =
      reinterpret_cast<const MyGL::Camera&>( scene.camera() );
  float scaleSize = 0.3/std::max( view.distance(), 1.0 )/3.0;

  out = localTex.create( w, h, MyGL::Texture2d::Format::Luminance8 );
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

void GraphicsSystem::aceptGI(   const Scene & s,
                                MyGL::Texture2d &out,
                                const MyGL::Texture2d &scene,
                                const MyGL::Texture2d &diff,
                                const MyGL::Texture2d &norm,
                                const MyGL::Texture2d &sdepth,
                                const MyGL::Texture2d gi[4] ) {
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

  const MyGL::AbstractCamera &camera = s.camera();
  MyGL::Matrix4x4 mat = camera.projective();
  mat.mul( camera.view() );
  mat.inverse();

  MyGL::DirectionLight light;
  if( s.lights().direction().size() > 0 )
    light = s.lights().direction()[0];

  double dir[3] = { light.xDirection(),
                    light.yDirection(),
                    light.zDirection() };
  MyGL::Matrix4x4 shMatrix = makeShadowMatrix(s, dir);

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

void GraphicsSystem::ssaoGMap( const Scene &scene,
                               MyGL::Texture2d &sm  ) {
  sm = localTex.create( 256, 256,
                        MyGL::AbstractTexture::Format::RGB10_A2 );

  MyGL::RenderState rstate;
  rstate.setCullFaceMode( MyGL::RenderState::CullMode::front );

  MyGL::Texture2d depthSm = depth( sm.width(), sm.height() );
  { const Scene::Objects &v = scene.shadowCasters();
    MyGL::Render render( device,
                         sm, depthSm,
                         smap.vs, smap.fs );
    render.clear( MyGL::Color(1.0), 1 );
    render.setRenderState( rstate );

    const MyGL::AbstractCamera & camera = scene.camera();

    double dir[] = {0,0, -1};
    MyGL::Matrix4x4 matrix = makeShadowMatrix(scene, dir);

    for( size_t i=0; i<v.size(); ++i ){
      const AbstractGraphicObject& ptr = *v[i];
      MyGL::Matrix4x4 m = matrix;

      if( !ptr.material().usage.blush )
        m.mul( ptr.transform() ); else
        m.mul( Material::animateObjMatrix( ptr.transform() ) );

      if( scene.viewTester().isVisible( ptr, m ) ){
        MyGL::UniformTable table( device, smap.vs, smap.fs );
        MyGL::RenderState rs;

        table.add( m, "mvpMatrix", MyGL::UniformTable::Vertex );
        ptr.material().shadow( rs, ptr.transform(), camera, table );

        device.setRenderState(rs);
        render.draw( ptr );
        }
      }
    }

  /*
  { const Scene::Objects &v = scene.objects<BlushShMaterial>();
    MyGL::Render render( device,
                         sm, depthSm,
                         smap.vs, smap.fs );
    render.setRenderState( rstate );

    const MyGL::AbstractCamera & camera = scene.camera();

    double dir[] = {0,0, -1};
    MyGL::Matrix4x4 matrix = makeShadowMatrix(scene, dir);


    for( size_t i=0; i<v.size(); ++i ){
      const MyGL::AbstractGraphicObject& ptr = v[i].object();
      MyGL::Matrix4x4 m = matrix;
      //m.mul( ptr.transform() );
      m.mul( BlushMaterial::animateObjMatrix( ptr.transform() ) );

      if( scene.viewTester().isVisible( ptr, m ) ){
        device.setUniform( smap.vs, m, "mvpMatrix" );
        render.draw( v[i].material(), ptr,
                     ptr.transform(), camera );
        }
      }
    }*/

  }

void GraphicsSystem::blurSm( MyGL::Texture2d &sm,
                             const Scene & scene ) {
  MyGL::Texture2d tmp = localTex.create( sm.width(), sm.height(),
                        MyGL::AbstractTexture::Format::RGB10_A2 );

  float s = 6*smMatSize(scene);
  gauss( tmp,  sm, sm.width(), sm.height(), s, 0 );
  gauss(  sm, tmp, sm.width(), sm.height(), 0, s );

  gauss_gb( tmp,  sm, sm.width(), sm.height(), s*2, 0 );
  gauss_gb(  sm, tmp, sm.width(), sm.height(), 0, s*2 );

  gauss_b( tmp,  sm, sm.width(), sm.height(),  s*3, 0 );
  gauss_b(  sm, tmp, sm.width(), sm.height(),  0, s*3 );

  MyGL::Texture2d::Sampler sampler = reflect;
  sampler.uClamp = MyGL::Texture2d::ClampMode::Clamp;
  sampler.vClamp = sampler.uClamp;

  sm.setSampler( sampler );
  }

void GraphicsSystem::buildRSM( Scene &scene,
                               MyGL::Texture2d gbuffer[4],
                               int shadowMapSize ) {
  for( int i=0; i<3; ++i ){
    gbuffer[i] = localTex.create( shadowMapSize, shadowMapSize,
                                  MyGL::Texture2d::Format::RGBA );
    }
  gbuffer[3] = localTex.create( shadowMapSize, shadowMapSize,
                                MyGL::Texture2d::Format::RG16 );
  MyGL::Texture2d mainDepth = depth( shadowMapSize, shadowMapSize );

  //MyGL::Camera &c = scene.camera();

  MyGL::DirectionLight light;
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

  MyGL::Texture2d rsmNo[4];
  renderScene( scene,
               c,
               gbuffer,
               mainDepth, rsmNo, 1, 0 );
  }

void GraphicsSystem::renderScene( const Scene &scene,
                                  const MyGL::AbstractCamera & camera,
                                  MyGL::Texture2d gbuffer[4],
                                  MyGL::Texture2d &mainDepth,

                                  MyGL::Texture2d rsm[4],
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

  MyGL::Texture2d shadowMapCl = localTex.create(shadowMapSize, shadowMapSize),
                  depth = this->depth(shadowMapSize, shadowMapSize);
  fillTranscurentMap(shadowMapCl, depth, scene);

  fillGBuf( gbuffer, mainDepth,
            shadowMap, shadowMapCl,
            scene, camera );

  MyGL::Texture2d topSm;
  ssaoGMap( scene, topSm );

  drawOmni( gbuffer, mainDepth, topSm, scene );

  //blt( shadowMap );
  MyGL::Texture2d sceneCopy;

  if( useAO ){
    MyGL::Texture2d ssaoTex;
    blurSm(topSm, scene);
    ssao( ssaoTex, gbuffer[3], topSm, scene );

    MyGL::Texture2d aoAcepted;

    if( rsm[0].width()>0 )
      aceptGI  ( scene, aoAcepted, gbuffer[0], gbuffer[1],
                                   gbuffer[2], gbuffer[3], rsm ); else
      aceptSsao( scene, aoAcepted, gbuffer[0], gbuffer[1], ssaoTex );

    gbuffer[0] = aoAcepted;
    }

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
                                     MyGL::Texture2d &out  ) {
  particles = &e;

  int w = out.width(),
      h = out.height();

  MyGL::Texture2d gbuffer[4], rsm[4];
  MyGL::Texture2d mainDepth = this->depth(w,h);

  for( int i=0; i<3; ++i ){
    gbuffer[i] = localTex.create( w, h,
                                  MyGL::Texture2d::Format::RGBA );
    }
  gbuffer[3] = localTex.create( w, h,
                                MyGL::Texture2d::Format::RG16 );

  renderScene( scene,
               scene.camera(),
               gbuffer, mainDepth, rsm, 256, 0 );

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

void GraphicsSystem::setFog(const MyGL::Pixmap &p) {
  fogView = localTex.create(p,0);
  }
