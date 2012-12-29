#ifndef GRAPHICSSYSTEM_H
#define GRAPHICSSYSTEM_H

#include <stdint.h>

#include <MyGL/DirectX9>
#include <MyGL/Device>

#include <MyGL/VertexBufferHolder>

#include <MyGL/IndexBufferHolder>

#include <MyGL/TextureHolder>
#include <MyGL/LocalTexturesHolder>

#include <MyGL/VertexShaderHolder>
#include <MyGL/FragmentShaderHolder>

#include <MyGL/GraphicObject>
#include <MyGL/Scene>

#include <MyGL/Algo/GBufferFillPass>
#include <MyGL/Algo/RenderAlgo>
#include <MyGL/Algo/ShadowMapPass>

#include <MyGL/Size>
#include <MyGL/Algo/PostProcessHelper>

#include "graphics/guipass.h"

namespace MyGL{
  class Camera;
  }

class Resource;
class MainGui;
class ParticleSystemEngine;

class GraphicsSystem {
  public:
    GraphicsSystem( void *hwnd, int w, int h, bool isFullScreen, int smSize );

    bool render(const MyGL::Scene &scene, ParticleSystemEngine &e,
                 size_t dt );
    void resizeEvent( int w, int h, bool isFullScreen );

    void load( Resource & r, MainGui & gui, int w, int h );

    void renderSubScene( const MyGL::Scene &scene,
                         MyGL::Texture2d & out );
  private:
    MyGL::DirectX9 directx;

    static MyGL::Device::Options makeOpt( bool isFullScreen );
  public:
    MyGL::Device   device;

    MyGL::TextureHolder        texHolder;
    MyGL::LocalTexturesHolder  localTex;
    MyGL::VertexBufferHolder   vboHolder;
    MyGL::IndexBufferHolder    iboHolder;

    MyGL::VertexShaderHolder   vsHolder;
    MyGL::FragmentShaderHolder fsHolder;

    MyGL::PostProcessHelper    ppHelper;


    struct ObjectsClosure{
      struct{
        MyGL::Matrix4x4 matrix;
        } shadow;

      } closure;
  private:
    GUIPass gui;
    MainGui * widget;

    MyGL::Size  screenSize;
    static  MyGL::Matrix4x4 makeShadowMatrix(const MyGL::Scene & s , double *dxyz);

    MyGL::Texture2d::Sampler reflect, bufSampler;

    void makeRenderAlgo( Resource &res,
                         MainGui &gui,
                         int w, int h );
    struct Sm{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } smap;

    struct GBuf{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;

      MyGL::VertexShader   terrainVs;
      MyGL::FragmentShader terrainFs;

      MyGL::Uniform<float[3]> lightDirection, lightColor, lightAblimient,
                              view;
      } gbuf;

    struct Transparent{
      MyGL::VertexShader   vs, vsAdd;
      MyGL::FragmentShader fs, fsAdd;
      } transparentData;

    struct Displace{
      MyGL::VertexShader   vs, vsWater;
      MyGL::FragmentShader fs, fsWater;
      } displaceData;

    struct Water{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;

      MyGL::Texture2d      waterHeightMap;
      } water;

    struct Glow{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } glowData;

    struct Bloom{
      MyGL::Uniform< MyGL::Texture2d > b[3];

      MyGL::VertexShader   vs;
      MyGL::FragmentShader brightPass, combine;
      } bloomData;

    struct Blt{
      MyGL::Uniform< MyGL::Texture2d > texture;
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } bltData;

    struct Gauss{
      MyGL::Uniform< MyGL::Texture2d > texture;
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } gaussData;

    struct Omni{
      MyGL::Uniform< MyGL::Texture2d > texture;
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } omniData;

    struct Final{
      MyGL::Uniform< MyGL::Texture2d > scene, bloom, glow;
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs, avatar;
      } finalData;

    struct SSAO{
      MyGL::Uniform< MyGL::Texture2d > texture, blured, macro;
      MyGL::Uniform< MyGL::Texture2d > scene, diff, ssao;
      MyGL::Uniform<float[3]> lightAblimient;

      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs, detail, accept;
      } ssaoData;

    MyGL::Uniform<float[2]> scrOffset, cpyOffset;

    void fillGBuf( MyGL::Texture2d *gbuffer,
                   MyGL::Texture2d &mainDepth,
                   const MyGL::Texture2d &sm,
                   const MyGL::Scene &scene);

    void drawOmni(MyGL::Texture2d *gbuffer,
                   MyGL::Texture2d &mainDepth, const MyGL::Scene &scene);

    void setupLight( const MyGL::Scene &scene,
                     MyGL::FragmentShader & fs ,
                     const MyGL::Texture2d &sm);

    void fillShadowMap( MyGL::Texture2d &sm,
                        const MyGL::Scene &scene );

    void fillShadowMap(MyGL::Texture2d &sm, MyGL::Texture2d &depth,
                        const MyGL::Scene &scene,
                       const MyGL::Scene::Objects &v, bool clr);

    void drawObjects( MyGL::Texture2d* gbuffer,
                      MyGL::Texture2d &mainDepth,
                      const MyGL::Scene &scene,
                      const MyGL::Scene::Objects &v, bool clr );

    void drawObjects( MyGL::VertexShader   vs,
                      MyGL::FragmentShader fs,

                      MyGL::Texture2d* gbuffer,
                      MyGL::Texture2d &mainDepth,
                      const MyGL::Scene &scene,
                      const MyGL::Scene::Objects &v,
                      bool clr = false );

    void drawTranscurent( MyGL::Texture2d &screen,
                          MyGL::Texture2d& mainDepth,
                          MyGL::Texture2d &sceneCopy,
                          const MyGL::Scene &scene,
                          const MyGL::Scene::Objects &v ) ;

    void drawWater( MyGL::Texture2d &screen,
                    MyGL::Texture2d& mainDepth,
                    MyGL::Texture2d &sceneCopy,
                    MyGL::Texture2d &sm,
                    MyGL::Texture2d& sceneDepth,
                    const MyGL::Scene &scene,
                    const MyGL::Scene::Objects &v ) ;

    void drawGlow(MyGL::Texture2d &out,
                   MyGL::Texture2d &depth, const MyGL::Scene &scene , int size);

    void copy( MyGL::Texture2d &out,
               const MyGL::Texture2d& in );
    void copyDepth(MyGL::Texture2d &out,
                    const MyGL::Texture2d& in , int w, int h);
    void copy( MyGL::Texture2d &out,
               const MyGL::Texture2d& in, int w, int h );
    void gauss( MyGL::Texture2d &out,
                const MyGL::Texture2d& in, int w, int h, float dx, float dy );

    void bloom( MyGL::Texture2d &out,
                const MyGL::Texture2d& in );
    void blt(const MyGL::Texture2d &tex);

    void waves( MyGL::Texture2d &out,
                const MyGL::Texture2d& in );

    void ssao(MyGL::Texture2d &out,
               const MyGL::Texture2d& in , const MyGL::Scene &scene);

    void aceptSsao( const MyGL::Scene &s,
                    MyGL::Texture2d &out,
                    const MyGL::Texture2d& scene,
                    const MyGL::Texture2d &diff,
                    const MyGL::Texture2d &ssao);

    void ssaoDetail( MyGL::Texture2d &out,
                     const MyGL::Texture2d& in , const MyGL::Texture2d &macro);

    void ssaoGMap( const MyGL::Scene &s,
                   MyGL::Texture2d &out );

    MyGL::Texture2d depth( int w, int h );
    MyGL::Texture2d depth( const MyGL::Size& sz );


    void renderScene(const MyGL::Scene &scene,
                      MyGL::Texture2d gbuffer[4],
                      MyGL::Texture2d & depthBuffer , int shadowMapSize, bool useAO);

    unsigned int time;

    friend class DisplaceMaterial;
    friend class GlowMaterial;
    friend class TransparentMaterial;
    friend class WaterMaterial;
  };


#endif // GRAPHICSSYSTEM_H
