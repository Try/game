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
    vboHolder( device ),
    iboHolder( device ),
    vsHolder ( device ),
    fsHolder ( device ),

    ppHelper( vboHolder, iboHolder ),

    gbuffer( texHolder, w, h ),
    shadowMap( texHolder, smSize, smSize ) {
  makeBuffers(w,h);

  //renderAlgo = makeRenderAlgo(w,h);
  closure.shadowMap = &shadowMap;
  }

MyGL::RenderAlgo GraphicsSystem::makeRenderAlgo( Resource &res,
                                                 MainGui &gui,
                                                 int w, int h ) {
    screenSize.w = w;
    screenSize.h = h;

    MyGL::RenderAlgo r;
    MyGL::VertexShader postProcessVS = vsHolder.load("./data/sh/postProcess.vert");

    glowData.vs = res.vshader("glow");
    glowData.fs = res.fshader("glow");

    transparentData.vs = res.vshader("blend_add");
    transparentData.fs = res.fshader("blend_add");

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
           fsHolder.load("./data/sh/blitShader.frag"),
           gbuffer.buffer(0),
           copyPassBind,

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

    return r;
  }

MyGL::RenderAlgo GraphicsSystem::makeGlowAlgo( Resource &, MainGui &,
                                               int /*w*/, int /*h*/ ) {
  MyGL::VertexShader postProcessVS = vsHolder.load("./data/sh/postProcess.vert");
  MyGL::FragmentShader gaussSh     = fsHolder.load("./data/sh/gauss.frag");
  MyGL::FragmentShader dwSampler   = fsHolder.load("./data/sh/dwSampler.frag");

  MyGL::RenderAlgo algo;

  algo
  << MyGL::makeMaterialPass< GlowMaterial >()
    ( GlowMaterial::exec,
      *this
      )

  << MyGL::makeTextureOperator
     ( ppHelper,
       postProcessVS,
       dwSampler,
       sceneCopy,
       downSamplerBind,

       glow,
       glowDS
       )

  << MyGL::makeTextureOperator
     ( ppHelper,
       postProcessVS,
       gaussSh,
       glow,
       gaussBindH,

       glowTmp,
       glowDS
       )

  << MyGL::makeTextureOperator
     ( ppHelper,
       postProcessVS,
       gaussSh,
       glowTmp,
       gaussBindV,

       glow,
       glowDS
       );

  return algo;
  }

MyGL::RenderAlgo GraphicsSystem::makeBloomAlgo(Resource &, MainGui &,
                                               int /*w*/, int /*h*/) {
  MyGL::VertexShader postProcessVS = vsHolder.load("./data/sh/postProcess.vert");
  MyGL::FragmentShader gaussSh     = fsHolder.load("./data/sh/gauss.frag");
  MyGL::FragmentShader dwSampler   = fsHolder.load("./data/sh/dwSampler.frag");
  MyGL::FragmentShader combine     = fsHolder.load("./data/sh/bloomCombine.frag");

  MyGL::RenderAlgo algo;

  for( int i=0; i<3; ++i ){
    algo
      << MyGL::makeTextureOperator
         ( ppHelper,
           postProcessVS,
           fsHolder.load("./data/sh/brightPass.frag"),
           *this,
           brightPassBind,

           bpRt,
           dwRtDS
           )

      << MyGL::makeTextureOperator
         ( ppHelper,
           postProcessVS,
           dwSampler,
           bpRt,
           downSamplerBind,

           dwRt[i],
           dwRtDS
           )

      << MyGL::makeTextureOperator
         ( ppHelper,
           postProcessVS,
           gaussSh,
           dwRt[i],
           gaussBindH,

           gaussTmp[i],
           dwRtDS
           )

      << MyGL::makeTextureOperator
         ( ppHelper,
           postProcessVS,
           gaussSh,
           gaussTmp[i],
           gaussBindV,

           dwRt[i],
           dwRtDS
           )

      << MyGL::makeTextureOperator
         ( ppHelper,
           postProcessVS,
           gaussSh,
           dwRt[i],
           gaussBindH,

           gaussTmp[i],
           dwRtDS
           )

      << MyGL::makeTextureOperator
         ( ppHelper,
           postProcessVS,
           gaussSh,
           gaussTmp[i],
           gaussBindV,

           bloom[i],
           dwRtDS
           );
    }

  algo << MyGL::makeTextureOperator
           ( ppHelper,
             postProcessVS,
             combine,
             *this,
             bloomeCombineBind,

             gaussTmp[0],
             dwRtDS
             );

  return algo;
  }

void GraphicsSystem::makeBuffers(int w, int h) {
  MyGL::Texture2d::Sampler bufSampler;
  bufSampler.uClamp = MyGL::Texture2d::ClampMode::ClampToBorder;
  bufSampler.vClamp = bufSampler.uClamp;

  MyGL::Texture2d::Sampler reflect;
  reflect.uClamp = MyGL::Texture2d::ClampMode::MirroredRepeat;
  reflect.vClamp = reflect.uClamp;

  finalImage    = MyGL::RenderTaget( texHolder, w, h );

  depthSmBuffer = texHolder.create( shadowMap.width(), shadowMap.height(),
                                    MyGL::Texture2d::Format::Depth24 );

  const int bpTexSize = 256;
  bpRt   = MyGL::RenderTaget( texHolder, bpTexSize, bpTexSize );
  bpRt.setSampler( bufSampler );

  for( int i=0, dwRtSize=bpTexSize; i<3; ++i ){
    dwRtSize /= 2;
    dwRtSize = std::max(1, dwRtSize);
    dwRt[i] = MyGL::RenderTaget( texHolder, dwRtSize, dwRtSize,
                                 MyGL::Texture2d::Format::RGB10_A2 );
    dwRt[i].setSampler( bufSampler );

    gaussTmp[i] = MyGL::RenderTaget( texHolder, dwRtSize, dwRtSize,
                                     MyGL::Texture2d::Format::RGB10_A2  );
    gaussTmp[i].setSampler( bufSampler );

    bloom[i] = MyGL::RenderTaget( texHolder, dwRtSize, dwRtSize,
                                  MyGL::Texture2d::Format::RGB10_A2  );
    }

  dwRtDS = texHolder.create( bpRt.width(), bpRt.height(),
                             MyGL::Texture2d::Format::Depth24 );

  sceneCopy = MyGL::RenderTaget( texHolder, gbuffer.width(), gbuffer.height() );
  sceneCopy.setSampler( reflect );
  sceneCopyDepth = texHolder.create( sceneCopy.width(), sceneCopy.height(),
                                     MyGL::Texture2d::Format::Depth24 );

  glowTmp = MyGL::RenderTaget( texHolder, w/2, h/2 );
  glow    = MyGL::RenderTaget( texHolder, w/2, h/2 );
  glowDS = texHolder.create( glow.width(), glow.height(),
                             MyGL::Texture2d::Format::Depth24 );
  glow.setSampler( bufSampler );
  glowTmp.setSampler( bufSampler );

  mainDepth = texHolder.create( w, h, MyGL::Texture2d::Format::Depth24 );

  waterNormals = MyGL::RenderTaget( texHolder, 512, 512 );
  wDepth       = texHolder.create( 512, 512,
                                   MyGL::Texture2d::Format::Depth24 );
  //waterNormals, wDepth, *waterHeightMap;
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

void GraphicsSystem::copyPassBind(const MyGL::Scene &,
                                  MyGL::Device &device,
                                  const MyGL::RenderTaget &rt,
                                  MyGL::VertexShader   &vs,
                                  MyGL::FragmentShader &fs ) {
  device.setUniform( fs, rt.surface(), "texture" );

  float v[2] = { 1.0f/rt.width(), 1.0f/rt.height() };
  device.setUniform( vs, v, 2, "dTexCoord");
  }

void GraphicsSystem::waterPassBind( const MyGL::Scene &,
                                    MyGL::Device &device,
                                    const MyGL::Texture2d &t,
                                    MyGL::VertexShader &vs,
                                    MyGL::FragmentShader &fs) {
  device.setUniform( fs, t, "texture" );

  static unsigned it = 0;
  it = (it+100)%10000;

  float ft[1] = { float(2*M_PI*it/10000.0) };

  device.setUniform( fs, ft, 1, "time");

  float v[2] = { 1.0f/512, 1.0f/512 };
  device.setUniform( vs, v, 2, "dTexCoord");
  }

void GraphicsSystem::render( const MyGL::Scene &scene, MyGL::Camera &camera ) {
    if( !device.startRender() )
      return;

    // scene.setCamera( camera );
    closure.shadow.matrix = makeShadowMatrix( scene );
    scene.render( renderAlgo, device );

    device.present();
    }

void GraphicsSystem::resizeEvent( int w, int h, bool isFullScreen ){
    screenSize.w = w;
    screenSize.h = h;

    device.reset( makeOpt(isFullScreen) );

    gbuffer   .resize(w,h);
    renderAlgo.resize(w,h);

    makeBuffers(w,h);
    }

MyGL::Device::Options GraphicsSystem::makeOpt(bool isFullScreen) {
    MyGL::Device::Options opt;
    opt.windowed = !isFullScreen;
    opt.vSync    = !opt.windowed;

    return opt;
    }

void GraphicsSystem::load( Resource &res, MainGui &gui, int w, int h ) {
    waterHeightMap = &res.texture("water/height");
    renderAlgo     = makeRenderAlgo( res, gui, w, h );
    }



void GraphicsSystem::downSamplerBind( const MyGL::Scene &,
                                      MyGL::Device &device,
                                      const MyGL::RenderTaget & input,
                                      MyGL::VertexShader &vs,
                                      MyGL::FragmentShader &fs ) {
    device.setUniform( fs, input.surface(), "texture" );

    float v[] = { 1.0f/input.width(), 1.0f/input.height() };
    device.setUniform( vs, v, 2, "dTexCoord");
    device.setUniform( fs, v, 2, "dTexCoord");
    }

void GraphicsSystem::brightPassBind( const MyGL::Scene &s,
                                     MyGL::Device &device,
                                     const GraphicsSystem &sys,
                                     MyGL::VertexShader &vs,
                                     MyGL::FragmentShader &fs ) {
    copyPassBind( s, device, sys.gbuffer.buffer(0), vs, fs );
    }

void GraphicsSystem::gaussBindH( const MyGL::Scene &,
                                 MyGL::Device &device,
                                 const MyGL::RenderTaget &input,
                                 MyGL::VertexShader &vs,
                                 MyGL::FragmentShader &fs ) {
    device.setUniform( fs, input.surface(), "texture" );

    float dt[] = { 1.0f/input.width(), 1.0f/input.height() };
    float v[] = { std::min(dt[0],dt[1]), 0 };
    device.setUniform( vs, dt, 2, "dTexCoord");
    device.setUniform( fs,  v, 2, "dTexCoord");
    }

void GraphicsSystem::gaussBindV( const MyGL::Scene &,
                                 MyGL::Device &device,
                                 const MyGL::RenderTaget &input,
                                 MyGL::VertexShader &vs,
                                 MyGL::FragmentShader &fs ) {
    device.setUniform( fs, input.surface(), "texture" );

    float dt[] = { 1.0f/input.width(), 1.0f/input.height() };
    float v[] = { 0, std::min(dt[0],dt[1]) };
    device.setUniform( vs, dt, 2, "dTexCoord");
    device.setUniform( fs,  v, 2, "dTexCoord");
    }

void GraphicsSystem::bloomeCombineBind( const MyGL::Scene &,
                                        MyGL::Device &device,
                                        const GraphicsSystem &sys,
                                        MyGL::VertexShader &vs,
                                        MyGL::FragmentShader &fs) {
    device.setUniform( fs, sys.bloom[0].surface(), "b0" );
    device.setUniform( fs, sys.bloom[1].surface(), "b1" );
    device.setUniform( fs, sys.bloom[2].surface(), "b2" );

    float v[] = { 1.0f/sys.screenSize.w, 1.0f/sys.screenSize.h };
    device.setUniform( vs, v, 2, "dTexCoord");
    }

void GraphicsSystem::finalPassBind( const MyGL::Scene &,
                                    MyGL::Device &device,
                                    const GraphicsSystem &sys,
                                    MyGL::VertexShader   & vs,
                                    MyGL::FragmentShader & fs ) {
    device.setUniform( fs, sys.gbuffer.buffer(0).surface(), "scene" );
    device.setUniform( fs, sys.gaussTmp[0].surface(), "bloom" );
    device.setUniform( fs, sys.glow.surface(),  "glow"  );

    float v[] = { 1.0f/sys.screenSize.w, 1.0f/sys.screenSize.h };
    device.setUniform( vs, v, 2, "dTexCoord");
    }
