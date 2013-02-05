#ifndef GRAPHICSSYSTEM_H
#define GRAPHICSSYSTEM_H

#include <stdint.h>

#include <MyGL/DirectX9>
#include <MyGL/Device>

#include <MyGL/VertexBufferHolder>
#include <MyGL/LocalVertexBufferHolder>

#include <MyGL/IndexBufferHolder>

#include <MyGL/TextureHolder>
#include <MyGL/LocalTexturesHolder>

#include <MyGL/VertexShaderHolder>
#include <MyGL/FragmentShaderHolder>

#include <MyGL/GraphicObject>
#include <MyGL/Scene>

#include <MyGL/Algo/RenderAlgo>
#include <MyGL/Algo/ShadowMapPass>

#include <MyGL/Size>
#include <MyGL/Algo/PostProcessHelper>

#include "graphics/guipass.h"
#include <MyWidget/signal>

namespace MyGL{
  class Camera;
  }

class Resource;
class MainGui;
class ParticleSystemEngine;

class GraphicsSystem {
  public:
    GraphicsSystem( void *hwnd, int w, int h, bool isFullScreen, int smSize );

    bool render(MyGL::Scene &scene,
                 ParticleSystemEngine &e, MyGL::Camera camera,
                 size_t dt );
    void resizeEvent( int w, int h, bool isFullScreen );

    void load( Resource & r, MainGui & gui, int w, int h );

    void renderSubScene( const MyGL::Scene &scene,
                         ParticleSystemEngine &e,
                         MyGL::Texture2d & out   );

    MyWidget::signal<> onRender;
    void setFog( const MyGL::Pixmap& p );
  private:
    MyGL::DirectX9 directx;

    static MyGL::Device::Options makeOpt( bool isFullScreen );
  public:
    MyGL::Device   device;

    MyGL::TextureHolder           texHolder;
    MyGL::LocalTexturesHolder     localTex;
    MyGL::VertexBufferHolder      vboHolder;
    MyGL::LocalVertexBufferHolder lvboHolder;
    MyGL::IndexBufferHolder       iboHolder;

    MyGL::VertexShaderHolder   vsHolder;
    MyGL::FragmentShaderHolder fsHolder;

    MyGL::PostProcessHelper    ppHelper;
    MyGL::Texture2d    fogView;


    struct ObjectsClosure{
      struct{
        MyGL::Matrix4x4 matrix;
        } shadow;

      } closure;
  private:
    GUIPass gui;
    MainGui * widget;
    ParticleSystemEngine * particles;

    bool useFog;

    MyGL::Size  screenSize;
    static  float smMatSize( const MyGL::Scene & s );
    static  MyGL::Matrix4x4 makeShadowMatrix(const MyGL::Scene & s , double *dxyz);

    MyGL::Texture2d::Sampler reflect, bufSampler;

    void makeRenderAlgo( Resource &res,
                         MainGui &gui,
                         int w, int h );
    void blurSm(MyGL::Texture2d &sm , const MyGL::Scene &scene);

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

      MyGL::Texture2d      waterHeightMap, envMap;
      } water;

    struct Glow{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } glowData;

    struct FogOfWar{
      MyGL::VertexShader   vs, vsAcept;
      MyGL::FragmentShader fs, fsAcept;
      } fogOfWar;

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
      MyGL::VertexShader   vs, vsGB, vsB;
      MyGL::FragmentShader fs, fsGB, fsB;
      } gaussData;

    struct Omni{
      MyGL::Uniform< MyGL::Texture2d > texture;
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } omniData;

    struct Volumetric{
      MyGL::Uniform< MyGL::Texture2d > scene;
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } volumetricData;

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
      MyGL::FragmentShader fs, detail, accept, acceptGI;
      } ssaoData;

    MyGL::Uniform<float[2]> scrOffset, cpyOffset;

    void fillGBuf( MyGL::Texture2d *gbuffer,
                   MyGL::Texture2d &mainDepth,
                   const MyGL::Texture2d &sm,
                   const MyGL::Scene &scene);

    void renderVolumeLight( const MyGL::Scene &scene,
                            MyGL::Texture2d &gbuffer,
                            MyGL::Texture2d &mainDepth,
                            MyGL::Texture2d &shadowMap );

    void drawOmni(MyGL::Texture2d *gbuffer,
                   MyGL::Texture2d &mainDepth, MyGL::Texture2d &sm, const MyGL::Scene &scene);

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

    void drawObjects(MyGL::VertexShader &vs,
                      MyGL::FragmentShader &fs,

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

    void aceptFog(MyGL::Texture2d &in_out, const MyGL::Texture2d &fog);

    void copy( MyGL::Texture2d &out,
               const MyGL::Texture2d& in );
    void copyDepth(MyGL::Texture2d &out,
                    const MyGL::Texture2d& in , int w, int h);
    void copy( MyGL::Texture2d &out,
               const MyGL::Texture2d& in, int w, int h );
    void gauss( MyGL::Texture2d &out,
                const MyGL::Texture2d& in,
                int w, int h, float dx, float dy );
    void gauss_gb( MyGL::Texture2d &out,
                   const MyGL::Texture2d& in,
                   int w, int h, float dx, float dy );
    void gauss_b( MyGL::Texture2d &out,
                  const MyGL::Texture2d& in,
                  int w, int h, float dx, float dy );

    void bloom( MyGL::Texture2d &out,
                const MyGL::Texture2d& in );
    void drawFogOfWar(MyGL::Texture2d &out , const MyGL::Scene &scene);

    void blt(const MyGL::Texture2d &tex);

    void waves( MyGL::Texture2d &out,
                const MyGL::Texture2d& in );

    void ssao(MyGL::Texture2d &out,
               const MyGL::Texture2d& in , const MyGL::Texture2d &gao, const MyGL::Scene &scene);

    void aceptSsao( const MyGL::Scene &s,
                    MyGL::Texture2d &out,
                    const MyGL::Texture2d& scene,
                    const MyGL::Texture2d &diff,
                    const MyGL::Texture2d &ssao);

    void aceptGI(const MyGL::Scene &s,
                  MyGL::Texture2d &out,
                  const MyGL::Texture2d& scene,
                  const MyGL::Texture2d &diff, const MyGL::Texture2d &norm, const MyGL::Texture2d &depth,
                  const MyGL::Texture2d gi[4] );

    void ssaoDetail( MyGL::Texture2d &out,
                     const MyGL::Texture2d& in , const MyGL::Texture2d &macro);

    void ssaoGMap( const MyGL::Scene &s,
                   MyGL::Texture2d &out );

    MyGL::Texture2d depth( int w, int h );
    MyGL::Texture2d depth( const MyGL::Size& sz );


    void renderScene(const MyGL::Scene &scene,
                      MyGL::Texture2d gbuffer[4],
                      MyGL::Texture2d & depthBuffer, MyGL::Texture2d rsm[],
                      int shadowMapSize, bool useAO);
    void buildRSM(MyGL::Scene &scene,
                   MyGL::Texture2d gbuffer[4],
                   int shadowMapSize, bool useAO);

    unsigned int time;

    friend class DisplaceMaterial;
    friend class GlowMaterial;
    friend class TransparentMaterial;
    friend class WaterMaterial;

    struct RSMCamera : public MyGL::AbstractCamera {
      RSMCamera(){
        p.identity();
        }

      MyGL::Matrix4x4 view() const {
        return v;
        }

      MyGL::Matrix4x4 projective() const{
        return p;
        }

      MyGL::Matrix4x4 v, p;
      };
  };


#endif // GRAPHICSSYSTEM_H
