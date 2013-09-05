#include "graphicssystem.h"

#include <Tempest/Model>
#include <Tempest/Texture2d>
#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>
#include <Tempest/Color>

#include <iostream>
#include <cmath>

#include <Tempest/GraphicObject>

#include "graphics/particlesystemengine.h"

#include <resource.h>
#include <cstdint>
#include <tuple>

struct GraphicsSystem::MaterialSort{
  static bool sm( const AbstractGraphicObject* xa,
                  const AbstractGraphicObject* xb ){
    const AbstractGraphicObject& a = *xa;
    const AbstractGraphicObject& b = *xb;

    size_t avbo = a.vboHandle();
    size_t bvbo = b.vboHandle();

    return avbo < bvbo;
    }

  static bool glow( const AbstractGraphicObject* xa,
                    const AbstractGraphicObject* xb ){
    const AbstractGraphicObject& a = *xa;
    const AbstractGraphicObject& b = *xb;

    size_t avbo = a.vboHandle(),
           atex = a.material().glow.handle();

    size_t bvbo = b.vboHandle(),
           btex = b.material().glow.handle();

    uint64_t idA = (uint64_t(avbo)<<32) + atex;
    uint64_t idB = (uint64_t(bvbo)<<32) + btex;


    return idA < idB;
    }

  static bool terrain( const AbstractGraphicObject* xa,
                       const AbstractGraphicObject* xb ){
    const AbstractGraphicObject& a = *xa;
    const AbstractGraphicObject& b = *xb;

    size_t avbo = a.vboHandle(),
           atex = a.material().diffuse.handle();

    size_t bvbo = b.vboHandle(),
           btex = b.material().diffuse.handle();

    uint64_t idA = (uint64_t(avbo)<<32) + atex;
    uint64_t idB = (uint64_t(bvbo)<<32) + btex;


    return idA < idB;
    }

  static bool less( const AbstractGraphicObject* xa,
                    const AbstractGraphicObject* xb ){
    const AbstractGraphicObject& a = *xa;
    const AbstractGraphicObject& b = *xb;

    size_t avbo = a.vboHandle(),
           atex = a.material().diffuse.handle();

    float ar = a.material().teamColor->r(),
          ag = a.material().teamColor->g(),
          ab = a.material().teamColor->b();

    size_t bvbo = b.vboHandle(),
           btex = b.material().diffuse.handle();

    float br = a.material().teamColor->r(),
          bg = a.material().teamColor->g(),
          bb = a.material().teamColor->b();

    return std::tie(avbo, atex, ar, ag, ab) <
           std::tie(bvbo, btex, br, bg, bb);

    uint64_t idA = (uint64_t(avbo)<<32) + atex;
    uint64_t idB = (uint64_t(bvbo)<<32) + btex;


    return idA < idB;
    }
  };

const float GraphicsSystem::oclusionMSz = 0.6;

GraphicsSystem::GraphicsSystem( void *hwnd,
                                bool isFullScreen )
  : api( createAPI() ),
    device( *api, makeOpt(isFullScreen), hwnd ),
    texHolder ( device ),
    localTex  ( device ),
    vboHolder ( device ),
    lvboHolder( device ),
    iboHolder ( device ),
    liboHolder( device ),
    vsHolder  ( device ),
    fsHolder  ( device ) {
  widget    = 0;
  time      = -1;
  particles = 0;
  resource  = 0;

  nFrame = 0;
  useDirectRender = false;

  useFog    = false;
  useHDR    = false;

  toDraw.reserve(2048);

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
  }

void GraphicsSystem::makeRenderAlgo( int w, int h ) {
  Resource &res = *resource;
  setupScreenSize(w,h);

  gbuf.vs     = res.vshader("unit_main_material");
  gbuf.fs     = res.fshader("unit_main_material");

  gbuf.vsA    = res.vshader("main_material_atest");
  gbuf.fsA    = res.fshader("main_material_atest");

  gbuf.zfillVs   = res.vshader("zfill_material");
  gbuf.zfillFs   = res.fshader("zfill_material");

  gbuf.grassVs   = res.vshader("grass_material");
  gbuf.grassFs   = res.fshader("grass_material");

  gbuf.terrainVs      = res.vshader("terrain_main_material");
  gbuf.terrainFs      = res.fshader("terrain_main_material");

  gbuf.terrainMinorVs = res.vshader("terrain_minor_main_material");
  gbuf.terrainMinorFs = res.fshader("terrain_minor_main_material");

  //gbuf.lightDirection.setName("lightDirection");
  //gbuf.lightColor    .setName("lightColor");
  //gbuf.lightAblimient.setName("lightAblimient");
  //gbuf.view          .setName("view");

  smap.vs = res.vshader("shadow_map");
  smap.fs = res.fshader("shadow_map");

  smap.vsA = res.vshader("shadow_map_atest");
  smap.fsA = res.fshader("shadow_map_atest");

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

  //gaussData.texture.setName("texture");
  gaussData.vs = res.vshader("gauss");
  gaussData.fs = res.fshader("gauss");

  gaussData.vsAO = res.vshader("gauss_ao");
  gaussData.fsAO = res.fshader("gauss_ao");

  gaussData.vsGB = res.vshader("gauss_gb");
  gaussData.fsGB = res.fshader("gauss_gb");

  gaussData.vsB = res.vshader("gauss_b");
  gaussData.fsB = res.fshader("gauss_b");

  fogOfWar.vs = res.vshader("fog_of_war");
  fogOfWar.fs = res.fshader("fog_of_war");

  glowData.vs = res.vshader("glow");
  glowData.fs = res.fshader("glow");

  Tempest::Pixmap pm(1,1, false);
  pm.set(0,0, Tempest::Pixmap::Pixel{0,0,0,0});
  glowData.black1x1 = localTex.create(pm);

  fogOfWar.vsAcept = gaussData.vs;
  fogOfWar.fsAcept = res.fshader("aceptFog");

#ifndef __ANDROID__
  volumetricData.vs = gaussData.vs;
  volumetricData.fs = res.fshader("volumetricLight");
#endif

  bloomData.vs         = res.vshader("brightPass");
  bloomData.brightPass = res.fshader("brightPass");

  bloomData.combine    = res.fshader("bloomCombine");
  //bloomData.b[0].setName("b0");
  //bloomData.b[1].setName("b1");
  //bloomData.b[2].setName("b2");

  water.waterHeightMap[0] = texHolder.load("data/textures/water/hmap.png");
  water.waterHeightMap[1] = texHolder.load("data/textures/water/hmap1.png");

  water.envMap = res.texture("sky0/diff");

  water.vs     = res.vshader("htonorm");
  water.fs     = res.fshader("htonorm");

  finalData.vs     = res.vshader("final");
  finalData.fs     = res.fshader("final");
  finalData.avatar = res.fshader("avatar_final");

  //finalData.scene.setName("scene");
  //finalData.bloom.setName("bloom");
  //finalData.glow .setName("glow");

#ifndef __ANDROID__
  omniData.vs = res.vshader("omni");
  omniData.fs = res.fshader("omni");
#endif

  //scrOffset.setName("dTexCoord");
  //cpyOffset = scrOffset;

  //bltData.texture.setName("texture");
  bltData.vs = res.vshader("blt");
  bltData.fs = res.fshader("blt");
  
#ifndef __ANDROID__
  ssaoData.vs       = gaussData.vs;
  ssaoData.acceptGI = res.fshader("gi_accept");
#endif

  //ssaoData.texture.setName("texture");
  //ssaoData.blured. setName("blured");
  //ssaoData.macro.  setName("macro");
  //ssaoData.scene  .setName("scene");
  //ssaoData.diff   .setName("diff");
  //ssaoData.ssao.   setName("ssao");
  //ssaoData.lightAblimient.setName("lightAblimient");

  reflect.uClamp = Tempest::Texture2d::ClampMode::MirroredRepeat;
  reflect.vClamp = reflect.uClamp;

  bufSampler.uClamp = Tempest::Texture2d::ClampMode::ClampToBorder;
  bufSampler.vClamp = bufSampler.uClamp;
  }

float GraphicsSystem::smMatSize( const Scene &scene, float sv, float maxSv ) {
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

Tempest::Matrix4x4 GraphicsSystem::makeShadowMatrix( const Scene & scene ){
  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  float dir[3] = { float(light.xDirection()),
                   float(light.yDirection()),
                   float(light.zDirection()) };

  return makeShadowMatrix( scene,dir,0 );
  }

Tempest::Matrix4x4 GraphicsSystem::makeShadowMatrix( const Scene & scene,
                                                     float * dir,
                                                     float sv,
                                                     bool aspect ){
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

  float m[4*4] = {
     s, 0,  0, 0,
     0, s,  0, 0,
     s*dir[0], s*dir[1], cs*dir[2], 0,
     -s*x+s*z, -s*y, s*z+dist, 1,
     };

  mat.setData( m );

  Tempest::Matrix4x4 proj;

  if( aspect ){
    float a = screenSize.h/float(screenSize.w);
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

void GraphicsSystem::resizeEvent( int w, int h, bool isFullScreen ){
  setupScreenSize(w,h);
  device.reset( makeOpt(isFullScreen) );
  }

Tempest::Device::Options GraphicsSystem::makeOpt(bool isFullScreen) {
  Tempest::Device::Options opt;
  opt.windowed = !isFullScreen;
  opt.vSync    = !opt.windowed;

  return opt;
  }

void GraphicsSystem::load(Resource &res, MainGui &gui, int w, int h ) {
  widget   = &gui;
  resource = &res;
  //waterHeightMap = &res.texture("water/height");
  makeRenderAlgo( w, h );
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
  toDraw.clear();

  if( !device.startRender() )
    return false;
  ++nFrame;

  onRender( std::max<size_t>(dt-time, 0) );
  //gui->update( *widget, device );

  time = dt;//(time+dt);
  unsigned tx = time%(16*1024);

  Material::wind = sin( 2.0*M_PI*tx/(16*1024.0) );

  particles = &par;

  Tempest::Texture2d gbuffer[4], rsm[4];
  Tempest::Texture2d mainDepth;

  if( !useDirectRender ){
    mainDepth = depth( screenSize );

    gbuffer[0] = colorBuf( screenSize.w, screenSize.h );

  #ifndef __ANDROID__
    for( int i=1; i<3; ++i ){
      gbuffer[i] = colorBuf( screenSize.w, screenSize.h );
      }

    if( GraphicsSettingsWidget::Settings::api
        == GraphicsSettingsWidget::Settings::openGL){
      gbuffer[3] = colorBuf( screenSize.w, screenSize.h );
      } else {
      if( useHDR )
        gbuffer[3] = localTex.create( screenSize.w, screenSize.h,
                                      Tempest::Texture2d::Format::RGBA16 ); else
        gbuffer[3] = localTex.create( screenSize.w, screenSize.h,
                                      Tempest::Texture2d::Format::RG16 );
      }
  #endif
    }

  //scrOffset.set( 1.0f/screenSize.w, 1.0f/screenSize.h );
  //buildRSM( scene, rsm, 512, 0 );
  scene.setCamera( camera );
  /*
  blt( rsm[0] );
  device.present();
  return 1;
  */

  renderScene( scene,
               scene.camera(),
               gbuffer,
               mainDepth,
               useDirectRender?0:4,
               rsm,
               settings.shadowMapRes,
               false );

  Tempest::Texture2d fog;
  if( fogView.width()>0 ){
    drawFogOfWar(fog, scene);
    aceptFog( gbuffer[0], fog );
    }

  //device.present();
  //return 1;

  static bool renderUI = true;
  if( widget && renderUI ){
    /*
    if( useDirectRender )
      gui->exec( *widget, 0, 0, device ); else
      gui->exec( *widget, gbuffer, &mainDepth, device );
      */
    }


  Tempest::Texture2d glow, bloomTex;

  drawGlow( glow, mainDepth,
            gbuffer[0],
            scene, 512,
            gbuffer[0].width(),
            gbuffer[0].height() );
  drawOpWindow();

  if( settings.glow ){
    if( fogView.width()>0 )
      aceptFog( glow, fog );
    }

  if( useDirectRender ){
    device.present();
    return 1;
    }

  if( settings.bloom )
    bloom( bloomTex, gbuffer[0] );

  {
    Tempest::Render render( device, finalData.vs, finalData.fs );
    render.setRenderState( Tempest::RenderState::PostProcess );

    //cpyOffset.set( 1.0/screenSize.w, 1.0/screenSize.h );
    if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
      float c[2] = { 1.0f/screenSize.w, 1.0f/screenSize.h };
      device.setUniform( finalData.vs, c, 2, "dTexCoord" );
      }

    device.setUniform( finalData.fs, gbuffer[0], "scene" );

    if( settings.bloom!=0 )
      device.setUniform( finalData.fs, bloomTex, "bloom" );
    if( settings.glow )
      device.setUniform( finalData.fs, glow, "glow"  );

    device.drawFullScreenQuad( finalData.vs, finalData.fs );
    }

  //blt( gbuffer[2] );
  //blt( mainDepth );
  //blt( waterWaves );
  //blt( fog );
  //blt( final );
  //blt( gao );
  //blt( ssaoTexDet );
  //blt( bloomTex );

  device.present();
  return 1;
  }


void GraphicsSystem::fillShadowMap( Tempest::Texture2d& shadowMap,
                                    const Scene & scene,
                                    bool clr ) {
  Tempest::Texture2d depthSm = depth( shadowMap.width(), shadowMap.height() );
  //Tempest::Texture2d::Sampler s = shadowMap.sampler();
  //s.anisotropic = false;
  //s.mipFilter   = Tempest::Texture2d::FilterType::Nearest;
  //shadowMap.setSampler( s );

  fillShadowMap( shadowMap, depthSm, scene, clr );
  }

void GraphicsSystem::fillShadowMap( Tempest::Texture2d& sm,
                                    Tempest::Texture2d& depthSm,
                                    const Scene & scene,
                                    bool /*clr*/ ) {
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

  particles->exec( matrix, proj, 0, true );
  //return;

  const Tempest::AbstractCamera & camera = scene.camera();

  Frustum frustum( matrix );

  {
  Tempest::Render render( device,
                          sm, depthSm,
                          smap.vsA, smap.fsA );
  if(1)
    render.clear( Tempest::Color(1.0), 1 ); else
    device.clearZ(1);

  render.setRenderState( rstate );

  draw( frustum,
        true,
        camera,
        scene.shadowCastersAtst(),
        &Material::shadow,
        ref(matrix) );
  completeDraw( render,
                camera,
                MaterialSort::sm,
                &Material::shadow,
                ref(matrix));
  }

  {
  Tempest::Render render( device,
                          sm, depthSm,
                          smap.vs, smap.fs );
  render.setRenderState( rstate );

  draw( frustum,
        true,
        camera,
        scene.shadowCasters(),
        &Material::shadow,
        ref(matrix) );

  if( !settings.transcurentShadows )
    draw( frustum,
          true,
          camera,
          scene.shadowCastersTransp(),
          &Material::shadow,
          ref(matrix) );

  completeDraw( render,
                camera,
                MaterialSort::sm,
                &Material::shadow,
                ref(matrix));
  }

  drawOpWindow();
  }

void GraphicsSystem::fillTranscurentMap( Tempest::Texture2d &sm,
                                         Tempest::Texture2d &depthSm,
                                         const Scene &scene,
                                         bool /*clr*/ ) {

  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  float dir[3] = { float(light.xDirection()),
                   float(light.yDirection()),
                   float(light.zDirection()) };
  {
  Tempest::Render render( device,
                          sm, depthSm,
                          transparentData.vsSh,
                          transparentData.fsSh );
  Tempest::Color cl = light.color();
  cl.set( cl.r(), cl.g(), cl.b(), 1 );
  render.clear( cl, 1 );
  }

  RSMCamera c;
  c.v = makeShadowMatrix( scene, dir );
  c.p.setData( 1, 0, 0, 0,
               0,-1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1 );
  setupLight( scene, transparentData.vsSh, transparentData.fsSh,
              0, 0, 0, false, false );

  drawObjects( transparentData.vsSh, transparentData.fsSh,
               &sm,
               &depthSm,
               1,
               scene,
               c,
               scene.shadowCastersTransp(),
               MaterialSort::sm,
               &Material::transparentShZ,
               false );

  drawObjects( transparentData.vsSh, transparentData.fsSh,
               &sm,
               &depthSm,
               1,
               scene,
               c,
               scene.shadowCastersTransp(),
               MaterialSort::less,
               &Material::transparentSh,
               false );
  }

void GraphicsSystem::setupLight( const Scene & scene,
                                 Tempest::VertexShader   & vs,
                                 Tempest::FragmentShader & fs,
                                 const Tempest::Texture2d * sm,
                                 const Tempest::Texture2d * smCl,
                                 const Tempest::Texture2d * ao,
                                 bool ldata,
                                 bool spec ){
  if( scene.lights().direction().size()>0 ){
    if( ldata ){
      Tempest::DirectionLight l = scene.lights().direction()[0];

      double ld[3] = {l.xDirection(), l.yDirection(), l.zDirection()};
      device.setUniform( fs, ld, 3, "lightDirection" );

      float lcl[4] = { l.color().r(), l.color().g(), l.color().b(), l.color().a() };
      device.setUniform( fs, lcl, 4, "lightColor" );

      float la[4] = { l.ablimient().r(), l.ablimient().g(), l.ablimient().b(), l.ablimient().a() };
      device.setUniform( fs, la, 4, "lightAblimient" );
      }

    if( spec ){
      Tempest::Matrix4x4 vm = scene.camera().view();

      float view[3] = { float(vm.at(2,0)),
                        float(vm.at(2,1)),
                        float(vm.at(2,2)) };
      float len = sqrt(view[0]*view[0] + view[1]*view[1] + view[2]*view[2]);

      for( int i=0; i<3; ++i )
        view[i] /= len;

      device.setUniform( fs, view, 3, "view" );
      }

    if( settings.shadowMapRes>0 && sm )
      device.setUniform( fs,    *sm, "shadowMap"    );

    if( settings.transcurentShadows && smCl )
      device.setUniform( fs,  *smCl, "shadowMapCl"  );

    if( settings.oclusion && ao ){
      device.setUniform( fs,    *ao, "oclusionMap"  );

      float dir[] = {0,0, -1};
      Tempest::Matrix4x4 matrix = makeShadowMatrix(scene, dir, 1.0*oclusionMSz, false );
      device.setUniform( vs, matrix, "oclusionMapMatrix"  );
      }

    if( settings.shadowMapRes>0 )
      device.setUniform( vs, makeShadowMatrix(scene), "shadowMatrix"  );
    }
  }

void GraphicsSystem::fillGBuf( Tempest::Texture2d* gbuffer,
                               Tempest::Texture2d& mainDepth,
                               const Tempest::Texture2d& sm,
                               const Tempest::Texture2d& smCl,
                               const Tempest::Texture2d& ao,
                               const Scene & scene,
                               const Tempest::AbstractCamera & camera,
                               bool gcall ) {
  //return;

  int gbuffSize = 4;
  if( GraphicsSettingsWidget::Settings::api==GraphicsSettingsWidget::Settings::openGL )
    gbuffSize = 1;

  if( !gbuffer )
    gbuffSize = 0;

  setupLight( scene, gbuf.terrainVs,      gbuf.terrainFs,      &sm, &smCl, &ao );
  setupLight( scene, gbuf.vs,             gbuf.fs,             &sm, &smCl, &ao );
  setupLight( scene, gbuf.vsA,            gbuf.fsA,            &sm, &smCl, &ao );
  setupLight( scene, gbuf.terrainMinorVs, gbuf.terrainMinorFs, &sm, &smCl, &ao );
  setupLight( scene, gbuf.grassVs,        gbuf.grassFs,        &sm, &smCl, &ao );
  setupLight( scene, transparentData.vs,  transparentData.fs,  &sm, &smCl, &ao );

  particles->exec( scene.camera().view(),
                   scene.camera().projective(),
                   1 );

  drawObjects( gbuf.zfillVs,
               gbuf.zfillFs,
               gbuffer,
               &mainDepth,
               gbuffSize,
               scene,
               camera,
               scene.terrainMinorObjects(),
               MaterialSort::sm,
               &Material::terrainMinorZ,
               true,
               true );

  if( gbuffSize>1 ){
    Tempest::Render r( device,
                       gbuffer,
                       gbuffSize,
                       gbuf.zfillVs,
                       gbuf.zfillFs );
    r.clear( Tempest::Color(0) );
    }

  drawObjects( gbuf.vsA,
               gbuf.fsA,
               gbuffer,
               &mainDepth,
               gbuffSize,
               scene,
               camera,
               scene.mainObjectsAtest(),
               MaterialSort::less,
               &Material::gbuffer,
               false );

  drawObjects( gbuffer,
               &mainDepth,
               gbuffSize,
               scene,
               camera,
               scene.mainObjects(),
               MaterialSort::less,
               &Material::gbuffer,
               false );

  if(1){
    drawObjects( gbuf.terrainMinorVs,
                 gbuf.terrainMinorFs,
                 gbuffer,
                 &mainDepth,
                 gbuffSize,
                 scene,
                 camera,
                 scene.terrainMinorObjects(),
                 MaterialSort::terrain,
                 &Material::terrainMinor,
                 false );
    }

  if(1){
    int count =
    drawObjects( gbuf.terrainVs,
                 gbuf.terrainFs,
                 gbuffer,
                 &mainDepth,
                 gbuffSize,
                 scene,
                 camera,
                 scene.terrainObjects(),
                 MaterialSort::terrain,
                 &Material::terrainMain,
                 false );
    (void)count;
    }

  drawObjects( gbuf.grassVs,
               gbuf.grassFs,
               gbuffer,
               &mainDepth,
               gbuffSize,
               scene,
               camera,
               scene.grassObjects(),
               MaterialSort::less,
               &Material::grass );

  drawObjects( transparentData.vsAdd,
               transparentData.fsAdd,
               gbuffer,
               &mainDepth,
               1,
               scene,
               camera,
               scene.additiveObjects(),
               MaterialSort::less,
               &Material::additive );

  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer,
               &mainDepth,
               1,
               scene,
               camera,
               scene.transparentObjects(),
               MaterialSort::less,
               &Material::transparentZ,
               false );

  drawObjects( transparentData.vs, transparentData.fs,
               gbuffer,
               &mainDepth,
               1,
               scene,
               camera,
               scene.transparentObjects(),
               MaterialSort::less,
               &Material::transparent,
               false );

  if( gcall )
    {}//drawOpWindow();
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

    if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
      float c[2] = { 1.0f/vlTex.width(), 1.0f/vlTex.height() };
      device.setUniform( volumetricData.vs, c, 2, "dTexCoord" );
      }

    device.drawFullScreenQuad( volumetricData.vs, volumetricData.fs );
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

    if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
      float c[2] = {1.0f/gbuffer.width(), 1.0f/gbuffer.height()};
      device.setUniform( bltData.vs, c, 2, "dTexCoord" );
      }
    device.setUniform( bltData.fs, vlTex, "texture" );

    device.drawFullScreenQuad( bltData.vs, bltData.fs );
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

  float dir[3] = {0,0,-1};
  //Tempest::Matrix4x4 shM = makeShadowMatrix(scene,dir);
  Tempest::Matrix4x4 shM = makeShadowMatrix(scene, dir, 1.0*oclusionMSz, false );


  float tc[] = { 1.0f/gbuffer[3].width(), 1.0f/gbuffer[3].height() };
  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL )
    device.setUniform( omniData.fs, tc, 2, "dTexCoord");

  const Tempest::AbstractCamera & camera = scene.camera();
  const Scene::Objects &v = scene.omni();

  Frustum frustum( camera );

  draw( frustum,
        true,
        camera,
        v,
        &Material::omni,
        shM );
  completeDraw( render,
                camera,
                MaterialSort::less,
                &Material::omni,
                shM );
  }

int GraphicsSystem::drawObjects( Tempest::Texture2d* gbuffer,
                                  Tempest::Texture2d* mainDepth,
                                  int bufC,
                                  const Scene &scene,
                                  const Tempest::AbstractCamera & camera,
                                  const Scene::Objects &v,
                                  bool (*cmp)( const AbstractGraphicObject*,
                                               const AbstractGraphicObject* ),
                                  void (Material::*func)( Tempest::RenderState& /*d*/,
                                                          const Tempest::Matrix4x4 & /*object*/,
                                                          const Tempest::AbstractCamera&,
                                                          Tempest::UniformTable & ) const,
                                  bool clr ) {
  return drawObjects( gbuf.vs, gbuf.fs, gbuffer, mainDepth,
                      bufC, scene, camera, v, cmp, func, clr );
  }

int GraphicsSystem::drawObjects( Tempest::VertexShader   & vs,
                                 Tempest::FragmentShader & fs,
                                 Tempest::Texture2d* gbuffer,
                                 Tempest::Texture2d* mainDepth,
                                 int bufC,
                                 const Scene &,
                                 const Tempest::AbstractCamera& camera,
                                 const Scene::Objects &v,
                                 bool (*cmp)( const AbstractGraphicObject*,
                                              const AbstractGraphicObject* ),
                                 void (Material::*func)( Tempest::RenderState& /*d*/,
                                                         const Tempest::Matrix4x4 & /*object*/,
                                                         const Tempest::AbstractCamera&,
                                                         Tempest::UniformTable & ) const,
                                 bool clr,
                                 bool clrDepth ) {
  toDraw.clear();
  if( bufC>0 && gbuffer ){
    Frustum frustum( camera );
    int r = draw( frustum, true, camera, v, func );

    if( r || clr ){
      Tempest::Render render( device,
                              gbuffer,
                              bufC,
                              *mainDepth,
                              vs, fs );
      if( clr ){
        if( clrDepth )
          render.clear( Tempest::Color(0.0), 1 ); else
          render.clear( Tempest::Color(0.0) );
        }

      completeDraw( render, camera, cmp, func );
      }
    return r;
    } else {
    Frustum frustum( camera );
    int r = draw( frustum, true, camera, v, func );

    if( r || clr ){
      Tempest::Render render( device, vs, fs );
      if( clr ){
        if( clrDepth )
          render.clear( Tempest::Color(0.0), 1 ); else
          render.clear( Tempest::Color(0.0) );
        }

      completeDraw( render, camera, cmp, func );
      }
    return r;
    }
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
  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL )
    device.setUniform( displaceData.fs, tc, 2, "dTexCoord");

  const Tempest::AbstractCamera & camera = scene.camera();

  Frustum frustum( camera );
  draw( frustum, true, camera, v, &Material::displace );
  completeDraw( render,
                camera,
                MaterialSort::less,
                &Material::displace );
  }

template< class ... Args, class ... FArgs >
int GraphicsSystem::draw(  const Frustum &frustum,
                           bool deepVTest,
                           const Tempest::AbstractCamera & camera,
                           const Scene::Objects & v,
                           void (Material::*func)( Tempest::RenderState& /*d*/,
                                                   const Tempest::Matrix4x4 & /*object*/,
                                                   const Tempest::AbstractCamera&,
                                                   Tempest::UniformTable &,
                                                   FArgs ... args ) const,
                           Args... args ){
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

            GraphicsSystem::VisibleRet ret = FullVisible;
            if( deepVTest )
              ret = isVisible( t.x+dpos, t.y+dpos, t.z+dpos, t.r, frustum );

            if( ret ){
              c+=draw( frustum, ret!=FullVisible, camera, t, func, args... );
              }
            }
          }

  return c;
  }

template< class ... Args, class ... FArgs >
void GraphicsSystem::completeDraw( Tempest::Render  & render,
                                   const Tempest::AbstractCamera & camera,
                                   bool (*cmp)( const AbstractGraphicObject*,
                                                const AbstractGraphicObject* ),
                                   void (Material::*func)( Tempest::RenderState& /*d*/,
                                                          const Tempest::Matrix4x4 & /*object*/,
                                                          const Tempest::AbstractCamera&,
                                                          Tempest::UniformTable &,
                                                          FArgs ... args ) const,
                                   Args... args ) {
  std::sort( toDraw.begin(), toDraw.end(), cmp );

  Tempest::UniformTable table( render );
  Tempest::RenderState rs;

  for( size_t i=0; i<toDraw.size(); ++i ){
    const AbstractGraphicObject& ptr = *toDraw[i];

    (ptr.material().*func)( rs, ptr.transform(), camera, table, args... );
    render.setRenderState(rs);
    render.draw( ptr );
    }

  toDraw.clear();
  drawCallFinished();
  }

void GraphicsSystem::drawWater( Tempest::Texture2d& screen,
                                Tempest::Texture2d& mainDepth,
                                Tempest::Texture2d& sceneCopy,
                                Tempest::Texture2d& sm,
                                Tempest::Texture2d& smCl,
                                Tempest::Texture2d& sceneDepth,
                                Tempest::Texture2d& ao,
                                const Scene &scene,
                                const Scene::Objects &v ) {

  Tempest::Texture2d waterWaves;
  waves( waterWaves, water.waterHeightMap[0], water.waterHeightMap[1] );

  setupLight( scene, displaceData.vsWater, displaceData.fsWater, &sm, &smCl, &ao );
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
  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL )
    device.setUniform( displaceData.fsWater, tc, 2, "dTexCoord");
  device.setUniform( displaceData.fsWater, waterWaves, "normalMap");
  device.setUniform( displaceData.fsWater, sceneDepth, "sceneDepth");

  float dc[] = {0,0};
  device.setUniform( displaceData.fsWater, dc, 2, "dWaterCoord");

  const Tempest::AbstractCamera & camera = scene.camera();

  Frustum frustum( camera );

  draw( frustum,
        true,
        camera,
        v,
        &Material::water  );
  completeDraw( render,
                camera,
                MaterialSort::less,
                &Material::water  );
  }

void GraphicsSystem::drawGlow( Tempest::Texture2d &out,
                               Tempest::Texture2d &depth,
                               Tempest::Texture2d &mrt0,
                               const Scene &scene,
                               int size,
                               int w, int h ) {
  int ps = 1;

  Tempest::Texture2d buffer = localTex.create( w,h, Tempest::Texture2d::Format::RGB );//colorBuf( w,h );

  int sz = std::max(w,h);
  while( ps*2< sz ){
    ps*=2;
    }

  if( size>ps )
    size = ps;

  if( !settings.glow ){
    out = glowData.black1x1;

    if( depth.width()>0 ){
      drawObjects( glowData.vs,
                   glowData.fs,
                   &mrt0,
                   &depth,
                   1,
                   scene,
                   scene.camera(),
                   scene.glowObjects(),
                   MaterialSort::glow,
                   &Material::glowPassAdd,
                   false,
                   false );
      }
    else {
      drawObjects( glowData.vs,
                   glowData.fs,
                   0,
                   0,
                   0,
                   scene,
                   scene.camera(),
                   scene.glowObjects(),
                   MaterialSort::glow,
                   &Material::glowPassAdd,
                   false,
                   false );
      }
    return;
    }

  const Tempest::AbstractCamera & camera = scene.camera();

  int c =
      drawObjects( glowData.vs, glowData.fs,
                   &buffer,
                   &depth,
                   1,
                   scene,
                   camera,
                   scene.glowObjects(),
                   MaterialSort::less,
                   &Material::glowPass,
                   true,
                   false );

  //int size = 512;
  if( c>0 ){
    Tempest::Texture2d tmp;
    //copy ( out, buffer,  size, size );

    copy ( tmp, buffer,  size, size );
    gauss( buffer, tmp,  size, size, 1.0, 0.0 );
    gauss( out,  buffer, size, size, 0.0, 1.0 );
    } else {
    out = glowData.black1x1;
    }

  //out = tmp;
  }

void GraphicsSystem::blt( const Tempest::Texture2d & tex ) {
  Tempest::Render render( device, bltData.vs, bltData.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = { 1.0f/screenSize.w, 1.0f/screenSize.h };
    device.setUniform( bltData.vs, c, 2, "dTexCoord" );
    }

  device.setUniform( bltData.fs, tex, "texture" );

  device.drawFullScreenQuad( bltData.vs, bltData.fs );
  }

void GraphicsSystem::copy( Tempest::Texture2d &out,
                           const Tempest::Texture2d& in ) {
  copy(out, in, in.width(), in.height() );
  }

void GraphicsSystem::copy( Tempest::Texture2d &out,
                           const Tempest::Texture2d& in,
                           int w, int h ) {
  copy( out, in.format(), in, w, h);
  }

void GraphicsSystem::copy( Tempest::Texture2d &out,
                           Tempest::Texture2d::Format::Type outFrm,
                           const Tempest::Texture2d& in,
                           int w, int h ) {
  out = localTex.create(w, h, outFrm);//colorBuf( w,h );
  out.setSampler( in.sampler() );

  Tempest::Render render( device,
                          out,
                          bltData.vs, bltData.fs );
  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/w, 1.0f/h };
    device.setUniform( bltData.vs, c, 2, "dTexCoord" );
    }
  device.setUniform( bltData.fs, in, "texture" );

  device.drawFullScreenQuad( bltData.vs, bltData.fs );
  }

void GraphicsSystem::copyDepth( Tempest::Texture2d &out,
                                const Tempest::Texture2d& in,
                                int w, int h ) {
  //int w = in.width(), h = in.height();

  out = localTex.create( w, h, Tempest::Texture2d::Format::RG16 );
  out.setSampler( reflect );

  Tempest::Render render( device,
                          out,
                          bltData.vs, bltData.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/w, 1.0f/h };
    device.setUniform( bltData.vs, c, 2, "dTexCoord" );
    }
  device.setUniform( bltData.fs, in, "texture" );

  device.drawFullScreenQuad( bltData.vs, bltData.fs );
  }

void GraphicsSystem::gauss( Tempest::Texture2d &out,
                            const Tempest::Texture2d& in,
                            int w, int h,
                            float dx, float dy ) {
  out = localTex.create( w,h, in.format() );
  //out.setSampler( in.sampler() );

  Tempest::Render render( device,
                          out,
                          gaussData.vs, gaussData.fs );
  //render.clear( Tempest::Color(0.0), 1 );

  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/w, 1.0f/h};
    device.setUniform( gaussData.vs, c, 2, "dTexCoord" );
    }

  //cpyOffset.set( dx/w, dy/h );
  float blurCoord[2] = { dx/w, dy/h };
  device.setUniform( gaussData.fs, blurCoord, 2, "blurCoord" );
  device.setUniform( gaussData.fs, in, "texture" );

  device.drawFullScreenQuad( gaussData.vs, gaussData.fs );
  }

void GraphicsSystem::gauss_ao( Tempest::Texture2d &out,
                               const Tempest::Texture2d &in,
                               int w, int h, float dx, float dy) {
  //out = localTex.create( w,h );
  //out.setSampler( in.sampler() );

  Tempest::Render render( device,
                          out,
                          gaussData.vsAO, gaussData.fsAO );

  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/w, 1.0f/h};
    device.setUniform( gaussData.vsAO, c, 2, "dTexCoord" );
    }

  float blurCoord[2] = { dx/w, dy/h };
  device.setUniform( gaussData.fsAO, blurCoord, 2, "blurCoord" );
  device.setUniform( gaussData.fsAO, in, "texture" );

  device.drawFullScreenQuad( gaussData.vsAO, gaussData.fsAO );
  }

void GraphicsSystem::gauss_gb( Tempest::Texture2d &out,
                               const Tempest::Texture2d& in,
                               int w, int h,
                               float dx, float dy ) {
  //out = localTex.create( w,h );
  //out.setSampler( reflect );

  Tempest::Render render( device,
                          out,
                          gaussData.vsGB, gaussData.fsGB );

  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/w, 1.0f/h};
    device.setUniform( gaussData.vsGB, c, 2, "dTexCoord" );
    }

  float blurCoord[2] = { dx/w, dy/h };
  device.setUniform( gaussData.fsGB, blurCoord, 2, "blurCoord" );
  device.setUniform( gaussData.fsGB, in, "texture" );

  device.drawFullScreenQuad( gaussData.vsGB, gaussData.fsGB );
  }

void GraphicsSystem::gauss_b( Tempest::Texture2d &out,
                               const Tempest::Texture2d& in,
                               int w, int h,
                               float dx, float dy ) {
  //out = localTex.create( w,h );
  //out.setSampler( reflect );

  Tempest::Render render( device,
                          out,
                          gaussData.vsB, gaussData.fsB );

  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/w, 1.0f/h};
    device.setUniform( gaussData.vsB, c, 2, "dTexCoord" );
    }

  float blurCoord[2] = { dx/w, dy/h };
  device.setUniform( gaussData.fsB, blurCoord, 2, "blurCoord" );
  device.setUniform( gaussData.fsB, in, "texture" );

  device.drawFullScreenQuad( gaussData.vsB, gaussData.fsB );
  }

void GraphicsSystem::bloom( Tempest::Texture2d &result,
                            const Tempest::Texture2d &in ) {
  //bloomData

#ifdef __ANDROID__
  Tempest::Size sizes[3] = {
    {128, 128}, {32, 32}, {16, 16}
    };
#else
  Tempest::Size sizes[3] = {
    {256, 256}, {64, 64}, {16, 16}
    };
#endif

  const int w = sizes[0].w, h = w;

  if( settings.bloom==GraphicsSettingsWidget::Settings::Off ){
    result = colorBuf(1,1);
    Tempest::Render render( device,
                            result,
                            bloomData.vs, bloomData.brightPass );
    render.clear( Tempest::Color(0) );
    return;
    }

  result = colorBuf( w,h );

  Tempest::Texture2d tmp[4];

  {
    tmp[0] = colorBuf( w,h );
    tmp[0].setSampler( reflect );

    Tempest::Render render( device,
                            tmp[0],
                            bloomData.vs, bloomData.brightPass );

    render.setRenderState( Tempest::RenderState::PostProcess );

    if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
      float c[2] = {1.0f/w, 1.0f/h};
      device.setUniform( bloomData.vs, c, 2, "dTexCoord" );
      }
    device.setUniform( bloomData.brightPass, in, "texture" );

    //ppHelper.drawFullScreenQuad( device, bloomData.vs, bloomData.brightPass );
    device.drawFullScreenQuad(bloomData.vs, bloomData.brightPass);
    }

  if( settings.bloom==GraphicsSettingsWidget::Settings::Low ){
    const Tempest::Size & sz = sizes[0];
    Tempest::Texture2d htmp;

    gauss( htmp,   tmp[0], sz.w, sz.h, 1.0, 0.0 );
    gauss( result,   htmp, sz.w, sz.h, 0.0, 1.0 );
    return;
    }

  for( int i=1; i<4; ++i ){
    Tempest::Texture2d  & out = tmp[i];
    const Tempest::Size & sz = sizes[i-1];

    Tempest::Texture2d htmp;

    gauss( htmp, tmp[i-1], sz.w, sz.h, 1.0, 0.0 );
    gauss(  out,     htmp, sz.w, sz.h, 0.0, 1.0 );
    out.setSampler( reflect );
    }

  tmp[0] = Tempest::Texture2d();

  { result.setSampler( reflect );

    Tempest::Render render( device,
                            result,
                            bloomData.vs, bloomData.brightPass );

    render.setRenderState( Tempest::RenderState::PostProcess );

    if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
      float c[2] = {};
      device.setUniform( bloomData.vs, c, 2, "dTexCoord" );
      }

    static const char* bn[] = {"b0", "b1", "b2"};
    for( int i=0; i<3; ++i ){
      device.setUniform( bloomData.combine, tmp[i+1], bn[i] );
      }

    device.drawFullScreenQuad( bloomData.vs, bloomData.combine );
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

    Frustum frustum( camera );
    draw( frustum, true, camera, v, &Material::fogOfWar, true );
    completeDraw( render, camera,
                  MaterialSort::less,
                  &Material::fogOfWar, true );
    draw( frustum, true, camera, v, &Material::fogOfWar, false );
    completeDraw( render, camera,
                  MaterialSort::less,
                  &Material::fogOfWar, false );
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

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/tmp.width(), 1.0f/tmp.height()};
    device.setUniform( fogOfWar.vs, c, 2, "dTexCoord" );
    }

  device.setUniform( fogOfWar.fsAcept, tmp, "scene" );
  device.setUniform( fogOfWar.fsAcept, fog, "fog"   );

  device.drawFullScreenQuad( fogOfWar.vsAcept, fogOfWar.fsAcept );
  }

void GraphicsSystem::waves( Tempest::Texture2d &out,
                            const Tempest::Texture2d& in,
                            const Tempest::Texture2d& in1 ) {
  int w = 2*in.width(), h = 2*in.height();

  out = localTex.create( w,h );
  //out.setSampler( reflect );

  Tempest::Render render( device,
                          out,
                          water.vs, water.fs );

  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/w, 1.0f/h};
    device.setUniform( water.vs, c, 2, "dTexCoord" );
    }

  device.setUniform( water.fs, in,  "texture" );
  device.setUniform( water.fs, in1, "texture1" );

  //sin( 2.0*M_PI*time/(4*1024.0) )
  //static unsigned time = 0;
  //++time;
  float t = 2*M_PI*float(time%2048)/2048.0f;

  device.setUniform ( water.fs, &t, 1, "time" );

  device.drawFullScreenQuad( water.vs, water.fs );
  }

void GraphicsSystem::aceptGI(   const Scene & s,
                                Tempest::Texture2d &out,
                                const Tempest::Texture2d &scene,
                                const Tempest::Texture2d &diff,
                                const Tempest::Texture2d &/*norm*/,
                                const Tempest::Texture2d &sdepth,
                                const Tempest::Texture2d gi[4] ) {
  //ssaoData.lightAblimient.set(1,1,1);

  int w = scene.width(), h = scene.height();

  out = colorBuf( w,h );
  out.setSampler( reflect );

  Tempest::Render render( device,
                          out,
                          ssaoData.vs, ssaoData.acceptGI );

  render.setRenderState( Tempest::RenderState::PostProcess );

  if( GraphicsSettingsWidget::Settings::api!=GraphicsSettingsWidget::Settings::openGL ){
    float c[2] = {1.0f/w, 1.0f/h};
    device.setUniform( ssaoData.vs, c, 2, "dTexCoord" );
    }

  const Tempest::AbstractCamera &camera = s.camera();
  Tempest::Matrix4x4 mat = camera.projective();
  mat.mul( camera.view() );
  mat.inverse();

  Tempest::DirectionLight l;
  if( s.lights().direction().size() > 0 )
    l = s.lights().direction()[0];

  float dir[3] = { float(l.xDirection()),
                   float(l.yDirection()),
                   float(l.zDirection()) };
  Tempest::Matrix4x4 shMatrix = makeShadowMatrix(s, dir);

  device.setUniform( ssaoData.acceptGI, mat,      "invMatrix" );
  device.setUniform( ssaoData.acceptGI, shMatrix, "shMatrix"  );
  shMatrix.inverse();
  device.setUniform( ssaoData.acceptGI, shMatrix, "invShMatrix"  );
  device.setUniform( ssaoData.acceptGI, sdepth,   "depth"     );

  device.setUniform( ssaoData.acceptGI, gi[2],   "ssaoN"    );
  device.setUniform( ssaoData.acceptGI, gi[3],   "ssaoD"     );
  device.setUniform( ssaoData.acceptGI, scene, "scene" );
  device.setUniform( ssaoData.acceptGI, diff,  "diff"  );
  device.setUniform( ssaoData.acceptGI, gi[0], "ssao"  );

  float la[4] = { l.ablimient().r(), l.ablimient().g(), l.ablimient().b(), l.ablimient().a() };
  device.setUniform( ssaoData.acceptGI, la, 4, "lightAblimient" );

  device.drawFullScreenQuad( ssaoData.vs, ssaoData.acceptGI );
  }

void GraphicsSystem::ssaoGMap( const Scene &scene,
                               Tempest::Texture2d &sm,
                               bool clr ) {
  sm = localTex.create( 512, 512,
                        Tempest::AbstractTexture::Format::RGB );

  Tempest::RenderState rstate;
  rstate.setCullFaceMode( Tempest::RenderState::CullMode::front );

  const Tempest::AbstractCamera & camera = scene.camera();

  float dir[] = {0,0, -1};
  Tempest::Matrix4x4 matrix = makeShadowMatrix(scene, dir, 1.0*oclusionMSz, false );

  Frustum frustum( matrix );

  Tempest::Texture2d depthSm = depth( sm.width(), sm.height() );
  { const Scene::Objects &v = scene.shadowCastersAtst();
    Tempest::Render render( device,
                            sm, depthSm,
                            smap.vsA, smap.fsA );
    if( clr )
      render.clear( Tempest::Color(1.0), 1 ); else
      device.clearZ(1);
    render.setRenderState( rstate );

    draw( frustum,
          true,
          camera,
          v,
          &Material::shadow,
          ref(matrix) );
    completeDraw( render,
                  camera,
                  MaterialSort::less,
                  &Material::shadow,
                  ref(matrix) );
    }
  { const Scene::Objects &v = scene.shadowCasters();
    Tempest::Render render( device,
                            sm, depthSm,
                            smap.vs, smap.fs );
    render.setRenderState( rstate );

    draw( frustum,
          true,
          camera,
          v,
          &Material::shadow,
          ref(matrix) );
    completeDraw( render,
                  camera,
                  MaterialSort::less,
                  &Material::shadow,
                  ref(matrix) );
    }
  }

Tempest::Texture2d GraphicsSystem::shadowMap(int w, int h) {
  if( GraphicsSettingsWidget::Settings::api
      == GraphicsSettingsWidget::Settings::openGL)
    return localTex.create(w,h, Tempest::AbstractTexture::Format::RGBA8 );

  return localTex.create(w,h, Tempest::AbstractTexture::Format::Luminance16 );
  }

Tempest::Texture2d GraphicsSystem::colorBuf(int w, int h) {
#ifdef __ANDROID__
  return localTex.create( w,h, Tempest::Texture2d::Format::RGB5 );
#endif
  if( useHDR )
    return localTex.create( w,h, Tempest::Texture2d::Format::RGBA16 ); else
    return localTex.create( w,h, Tempest::Texture2d::Format::RGBA8 );
  }

void GraphicsSystem::blurSm( Tempest::Texture2d &sm,
                             Tempest::Texture2d & out ) {
  Tempest::AbstractTexture::Format::Type frm = Tempest::AbstractTexture::Format::RGB10_A2;
#ifdef __ANDROID__
  frm = Tempest::AbstractTexture::Format::RGB;
#endif

  Tempest::Texture2d tmp = localTex.create( sm.width(), sm.height(), frm );
  out = localTex.create( sm.width(), sm.height(), frm );

  Tempest::Texture2d::Sampler sampler = reflect;
  sampler.uClamp = Tempest::Texture2d::ClampMode::Clamp;
  sampler.vClamp = sampler.uClamp;

  sm. setSampler( sampler );
  out.setSampler( sampler );
  tmp.setSampler( sampler );

  //float sb = smMatSize(scene);
  float s  = 1;//oclusionMSz*sb;

  gauss_ao( tmp,  sm, sm.width(), sm.height(), s, 0 );
  gauss_ao( out, tmp, sm.width(), sm.height(), 0, s );

  gauss_gb( tmp, out, sm.width(), sm.height(), s*2, 0 );
  gauss_gb( out, tmp, sm.width(), sm.height(), 0, s*2 );

  gauss_b( tmp, out, sm.width(), sm.height(),  s*3, 0 );
  gauss_b( out, tmp, sm.width(), sm.height(),  0, s*3 );
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

  float dir[3] = { float(light.xDirection()),
                   float(light.yDirection()),
                   float(light.zDirection()) };
  RSMCamera c;
  c.v = makeShadowMatrix( scene, dir );
  c.p.setData( 1, 0, 0, 0,
               0,-1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1 );

  // scene.setCamera( c );

  renderScene( scene,
               c,
               gbuffer,
               mainDepth,
               4,
               0,
               1,
               true );
  }

Tempest::AbstractAPI *GraphicsSystem::createAPI() {
#ifndef __ANDROID__
  if( GraphicsSettingsWidget::Settings::api ==
      GraphicsSettingsWidget::Settings::directX )
    return new Tempest::DirectX9();
#endif

  return new Tempest::Opengl2x();
  }

void GraphicsSystem::setupScreenSize(int w, int h) {
  guiSize.w = w;
  guiSize.h = h;

  //w = 512;
  //h = 512;

  screenSize.w = w;
  screenSize.h = h;

  potScreenSize.w = nextPot(w);
  potScreenSize.h = nextPot(h);
  }

GraphicsSystem::VisibleRet GraphicsSystem::isVisible( const AbstractGraphicObject &c,
                                                      const Frustum &frustum ) {
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

int GraphicsSystem::dipOptimizeRef() {
#ifdef TEMPEST_OPENGL
#ifdef __ANDROID__
  return 32*6;
#else
  return 32*6;
#endif
#else
  return 32*3;
#endif
  }

void GraphicsSystem::renderScene( const Scene &scene,
                                  const Tempest::AbstractCamera & camera,
                                  Tempest::Texture2d* gbuffer,
                                  Tempest::Texture2d &mainDepth,
                                  int gbufferSize,

                                  Tempest::Texture2d* rsm,
                                  int shadowMapSize,
                                  bool clrSm ) {
  //return;

  Tempest::DirectionLight light;
  if( scene.lights().direction().size() > 0 )
    light = scene.lights().direction()[0];

  float dir[3] = { float(light.xDirection()),
                   float(light.yDirection()),
                   float(light.zDirection()) };

  closure.shadow.matrix = makeShadowMatrix( scene, dir );

  Tempest::Texture2d shadowMap, shadowMapCl;

  if( settings.shadowMapRes>0 ){
    shadowMap = this->shadowMap( shadowMapSize, shadowMapSize );
    fillShadowMap( shadowMap, scene, clrSm );
    //blt(shadowMap);
    //return;
    }

  if( settings.shadowMapRes>0 &&
      settings.transcurentShadows ){
    shadowMapCl = this->colorBuf(shadowMapSize, shadowMapSize);
    Tempest::Texture2d depth = this->depth(shadowMapSize, shadowMapSize);
    fillTranscurentMap(shadowMapCl, depth, scene, clrSm);

    //blt(shadowMapCl);
    //return;
    }

  Tempest::Texture2d topSm, aoBlured;
  if( settings.oclusion ){
    ssaoGMap( scene, topSm, clrSm );
    Tempest::Texture2d tmp;
    copy(tmp, topSm, 256, 256);
    blurSm(tmp, aoBlured);

    //blt(aoBlured);
    //return;
    }

  if( gbufferSize==0 ){
    fillGBuf( 0, mainDepth,
              shadowMap, shadowMapCl,
              aoBlured,
              scene, camera,
              true );
    } else {
    fillGBuf( gbuffer, mainDepth,
              shadowMap, shadowMapCl,
              aoBlured,
              scene, camera,
              true );
    }

  if( GraphicsSettingsWidget::Settings::api==GraphicsSettingsWidget::Settings::openGL )
    return;//under construction

  drawOmni( gbuffer, mainDepth, topSm, scene );
  aoBlured = Tempest::Texture2d();

  //blt( shadowMap );

  if( rsm && rsm[0].width()>0 ){
    Tempest::Texture2d aoAcepted;

    if( rsm[0].width()>0 )
      aceptGI  ( scene, aoAcepted, gbuffer[0], gbuffer[1],
                                   gbuffer[2], gbuffer[3], rsm );
    gbuffer[0] = aoAcepted;
    }

  Tempest::Texture2d sceneCopy;
  copy( sceneCopy, gbuffer[0] );

  drawTranscurent( gbuffer[0], mainDepth, sceneCopy,
                   scene,
                   scene.displaceObjects() );

  drawWater( gbuffer[0],
             mainDepth,
             sceneCopy,
             shadowMap,
             shadowMapCl,
             gbuffer[3],
             aoBlured,
             scene,
             scene.waterObjects() );


  if( useFog )
    renderVolumeLight( scene,
                       gbuffer[0],
                       gbuffer[3],
                       shadowMap );
  }

void GraphicsSystem::renderSubScene( const Scene &scene,
                                     ParticleSystemEngine &e,
                                     Tempest::Texture2d &out  ) {
  particles = &e;

  int w = out.width(),
      h = out.height();

  Tempest::Texture2d gbuffer[4];
  Tempest::Texture2d mainDepth = this->depth(w,h);

  //scrOffset.set( 1.0f/w, 1.0f/h );

  gbuffer[0] = colorBuf( w, h );
#ifndef __ANDROID__
  for( int i=1; i<3; ++i ){
    gbuffer[i] = colorBuf( w, h );
    }

  if( useHDR )
    gbuffer[3] = localTex.create( w, h,
                                  Tempest::Texture2d::Format::RGBA16 ); else
    gbuffer[3] = localTex.create( w, h,
                                  Tempest::Texture2d::Format::RG16 );
#endif

  renderScene( scene,
               scene.camera(),
               gbuffer,
               mainDepth,
               4,
               0,
               256,
               true );

  Tempest::Texture2d glow;
  drawGlow( glow, mainDepth,
            gbuffer[0],
            scene,
            128,
            gbuffer[0].width(),
            gbuffer[0].height() );

  {
    Tempest::Render render( device,
                            out,
                            finalData.vs, finalData.avatar );

    render.setRenderState( Tempest::RenderState::PostProcess );

    {
      float c[2] = {1.0f/out.width(), 1.0f/out.height()};
      device.setUniform( finalData.vs, c, 2, "dTexCoord" );
      }

    device.setUniform( finalData.avatar, gbuffer[0], "scene" );

    if( settings.glow )
      device.setUniform( finalData.avatar, glow, "glow" );

    device.drawFullScreenQuad( finalData.vs, finalData.avatar );
    }

  //copy( out, glow );
  }

void GraphicsSystem::setFog(const Tempest::Pixmap &p) {
  fogView = localTex.create(p,0);
  }

void GraphicsSystem::setSettings(const GraphicsSettingsWidget::Settings &s) {
  settings           = s;
  Material::settings = s;

  makeRenderAlgo( screenSize.w, screenSize.h );

  useDirectRender = ( s.bloom==0 && s.glow==0 &&
                      GraphicsSettingsWidget::Settings::api==GraphicsSettingsWidget::Settings::openGL);
  }
