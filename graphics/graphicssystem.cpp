#include "graphicssystem.h"

#include <MyGL/Model>
#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include <MyGL/Algo/GBufferFillPass>
#include <MyGL/Algo/TextureOperator>
#include <MyGL/Algo/Blit>
#include <MyGL/MaterialPass>

#include <MyGL/Scene>

#include <iostream>
#include <cmath>

#include "graphics/guipass.h"
#include "graphics/displacematerial.h"
#include "graphics/glowmaterial.h"
#include "graphics/transparentmaterial.h"
#include "graphics/addmaterial.h"
#include "graphics/watermaterial.h"
#include "graphics/mainmaterial.h"
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

    gbuf.lightDirection.setName("lightDirection");
    gbuf.lightColor    .setName("lightColor");
    gbuf.lightAblimient.setName("lightAblimient");
    gbuf.view          .setName("view");

    smap.vs = vsHolder.load("./data/sh/shadow_map.vert");
    smap.fs = fsHolder.load("./data/sh/shadow_map.frag");

    transparentData.vs = res.vshader("blend_add");
    transparentData.fs = res.fshader("blend_add");

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

    finalData.vs = gaussData.vs;
    finalData.fs = fsHolder.load("./data/sh/final.frag");
    finalData.scene.setName("scene");
    finalData.bloom.setName("bloom");
    finalData.glow .setName("glow");

    scrOffset.setName("dTexCoord");
    scrOffset.set( 1.0f/screenSize.w, 1.0f/screenSize.h );

    cpyOffset = scrOffset;

    bltData.texture.setName("texture");
    bltData.vs = vsHolder.load("./data/sh/blitShader.vert");
    bltData.fs = fsHolder.load("./data/sh/blitShader.frag");

    reflect.uClamp = MyGL::Texture2d::ClampMode::MirroredRepeat;
    reflect.vClamp = reflect.uClamp;

    bufSampler.uClamp = MyGL::Texture2d::ClampMode::ClampToBorder;
    bufSampler.vClamp = bufSampler.uClamp;

    /*
    MyGL::RenderAlgo r;
    MyGL::VertexShader postProcessVS = vsHolder.load("./data/sh/postProcess.vert");

    glowData.vs = res.vshader("glow");
    glowData.fs = res.fshader("glow");


    displaceData.vs = res.vshader("displace");
    displaceData.fs = res.fshader("displace");

    displaceData.vsWater = res.vshader("water");
    displaceData.fsWater = res.fshader("water");

    r << MyGL::ShadowMapPass<MakeShadowMatrix>
          ( shadowMap, depthSmBuffer,
            vsHolder.load("./data/sh/shadow_map.vert"),
            fsHolder.load("./data/sh/shadow_map.frag"),
            makeShadowMatrix )

      << MyGL::GBufferFillPass( gbuffer,
                                mainDepth,
                                res.vshader("main_material"),
                                res.fshader("main_material")
                                )
      << MainPass( gbuffer,
                   mainDepth,
                   res.vshader("unit_main_material"),
                   res.fshader("unit_main_material")
                   )

      << MyGL::makeMaterialPass< AddMaterial >
        ( gbuffer.buffer(0),
          mainDepth,
          transparentData.vs,
          transparentData.fs
          )

      << MyGL::makeMaterialPass< TransparentMaterialZPass >
        ( sceneCopy,
          mainDepth,
          transparentData.vs,
          transparentData.fs
          )

      << MyGL::makeMaterialPass< TransparentMaterial >
        ( gbuffer.buffer(0),
          mainDepth,
          transparentData.vs,
          transparentData.fs
          )

      << MyGL::makeTextureOperator
         ( ppHelper,
           vsHolder.load("./data/sh/htonorm.vert"),
           fsHolder.load("./data/sh/htonorm.frag"),
           *waterHeightMap,
           waterPassBind,

           waterNormals,
           wDepth
           )

      << MyGL::makeTextureOperator
         ( ppHelper,
           vsHolder.load("./data/sh/blitShader.vert"),
           fsHolder.load("./data/sh/copyScr.frag"),
           *this,
           copyScrBind,

           sceneCopy,
           sceneCopyDepth
           )

      << MyGL::makeMaterialPass< DisplaceMaterial >()
        ( DisplaceMaterial::exec,
          *this
          )

      << MyGL::makeMaterialPass< WaterMaterial >()
        ( WaterMaterial::exec,
          *this
          )

      << GUIPass( gbuffer.buffer(0),
                  mainDepth,
                  res,
                  gui,
                  vsHolder,
                  fsHolder,
                  vboHolder,
                  screenSize )

      << makeBloomAlgo(res, gui, w, h)
      << makeGlowAlgo (res, gui, w, h)

      << MyGL::makeTextureOperator
         ( ppHelper,
           postProcessVS,
           fsHolder.load("./data/sh/final.frag"),
           *this,
           finalPassBind,

           finalImage
           )

      << MyGL::Blit ( ppHelper,
                      vsHolder.load("./data/sh/blitShader.vert"),
                      fsHolder.load("./data/sh/blitShader.frag"),
                      //gbuffer.buffer(0).surface(),
                      //shadowMap.shadowMap().surface(),
                      //bpRt.surface(),
                      //dwRt.surface(),
                      //gaussTmp.surface(),
                      //sceneCopy.surface(),
                      //glow.surface(),
                      //bloom.surface(),
                      //mainDepth,
                      //gaussTmp[0].surface(),
                      //waterNormals.surface(),
                      finalImage.surface(),
                      screenSize );

    return r;*/
  }

MyGL::Matrix4x4 GraphicsSystem::makeShadowMatrix( const MyGL::Scene & scene ){
    MyGL::Matrix4x4 mat;

    float dist = 0.4, x = 2, y = 2, s = 0.3, cs = 0.3;

    const MyGL::Camera &view =
        reinterpret_cast<const MyGL::Camera&>( scene.camera() );

    x = view.x();
    y = view.y();
    s /= std::max( view.distance(), 1.0 )/3.0;

    MyGL::DirectionLight light;
    if( scene.lights().direction().size() > 0 )
      light = scene.lights().direction()[0];

    double dir[3] = { light.xDirection(),
                      light.yDirection(),
                      light.zDirection() };

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

void GraphicsSystem::render( const MyGL::Scene &scene, MyGL::Camera &camera ) {
  if( !device.startRender() )
    return;

  //scene.setCamera( camera );
  closure.shadow.matrix = makeShadowMatrix( scene );

  MyGL::Texture2d gbuffer[4];
  MyGL::Texture2d mainDepth = depth( screenSize );

  for( int i=0; i<4; ++i ){
    gbuffer[i] = localTex.create( screenSize.w, screenSize.h,
                                  MyGL::Texture2d::Format::RGBA16 );
    }

  MyGL::Texture2d shadowMap = localTex.create( 1024, 1024,
                                               MyGL::AbstractTexture::Format::Luminance16 );
  fillShadowMap( shadowMap, scene );
  fillGBuf( gbuffer, mainDepth, shadowMap, scene );

  //blt( shadowMap );
  MyGL::Texture2d sceneCopy;
  copy( sceneCopy, gbuffer[0] );

  if( widget )
    gui.exec( *widget, gbuffer[0], mainDepth, device );

  drawTranscurent( gbuffer[0], mainDepth, sceneCopy,
                   scene,
                   scene.objects<DisplaceMaterial>());

  MyGL::Texture2d glow, bloomTex;
  drawGlow( glow, mainDepth, scene );

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
  blt( final );

  device.present();
  }

void GraphicsSystem::fillShadowMap( MyGL::Texture2d& sm,
                                    const MyGL::Scene & scene ) {
  const MyGL::Scene::Objects &v = scene.objects<MyGL::ShadowMapPassBase::Material>();

  MyGL::RenderState rstate;
  rstate.setCullFaceMode( MyGL::RenderState::CullMode::front );

  MyGL::Texture2d depthSm = depth( sm.width(), sm.height() );
  MyGL::Render render( device,
                       sm, depthSm,
                       smap.vs, smap.fs );
  render.clear( MyGL::Color(0.0), 1 );
  render.setRenderState( rstate );

  const MyGL::AbstractCamera & camera = scene.camera();
  MyGL::Matrix4x4 matrix = makeShadowMatrix(scene);

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

void GraphicsSystem::fillGBuf( MyGL::Texture2d* gbuffer,
                               MyGL::Texture2d& mainDepth,
                               const MyGL::Texture2d& sm,
                               const MyGL::Scene & scene ) {
  if( scene.lights().direction().size()>0 ){
    MyGL::FragmentShader & fs = gbuf.fs;

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
    device.setUniform( gbuf.fs, sm, "shadowMap" );
    }

  drawObjects( gbuffer, mainDepth,
               scene, scene.objects<MainMaterial>(), true );

  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer, mainDepth,
               scene, scene.objects<AddMaterial>() );

  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer, mainDepth,
               scene, scene.objects<TransparentMaterialZPass>() );

  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer, mainDepth,
               scene, scene.objects<TransparentMaterial>() );
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

void GraphicsSystem::drawGlow (MyGL::Texture2d &out,
                               MyGL::Texture2d &depth,
                               const MyGL::Scene &scene) {
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

  int size = 256;
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
