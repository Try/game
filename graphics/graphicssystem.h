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
#include <MyGL/AbstractScene>

#include <MyGL/Size>
#include <MyGL/Algo/PostProcessHelper>

#include "graphics/guipass.h"
#include "graphics/scene.h"

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

    bool render( Scene &scene,
                 ParticleSystemEngine &e, MyGL::Camera camera,
                 size_t dt );
    void resizeEvent( int w, int h, bool isFullScreen );

    void load( Resource & r, MainGui & gui, int w, int h );

    void renderSubScene( const Scene &scene,
                         ParticleSystemEngine &e,
                         MyGL::Texture2d & out   );

    MyWidget::signal<double> onRender;
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

    struct Frustum{
      float f[6][4];
      };

    static void mkFrustum( const MyGL::AbstractCamera& c, Frustum& out );
    static bool isVisible( const AbstractGraphicObject& c, const Frustum& f );
    static bool isVisible( float x, float y, float z, float r, const Frustum& f );
  private:
    GUIPass gui;
    MainGui * widget;
    ParticleSystemEngine * particles;

    bool useFog, useHDR;

    MyGL::Size  screenSize;
    static  float smMatSize( const Scene & s );
    static  MyGL::Matrix4x4 makeShadowMatrix( const Scene & s );
    static  MyGL::Matrix4x4 makeShadowMatrix( const Scene & s, double *dxyz );

    MyGL::Texture2d::Sampler reflect, bufSampler;

    void makeRenderAlgo( Resource &res,
                         MainGui &gui,
                         int w, int h );
    void blurSm(MyGL::Texture2d &sm , const Scene &scene);

    struct Sm{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } smap;

    struct GBuf{
      MyGL::VertexShader   vs, grassVs;
      MyGL::FragmentShader fs, grassFs;

      MyGL::VertexShader   terrainVs;
      MyGL::FragmentShader terrainFs;

      MyGL::Uniform<float[3]> lightDirection, lightColor, lightAblimient,
                              view;
      } gbuf;

    struct Transparent{
      MyGL::VertexShader   vs, vsAdd, vsSh;
      MyGL::FragmentShader fs, fsAdd, fsSh;
      } transparentData;

    struct Displace{
      MyGL::VertexShader   vs, vsWater;
      MyGL::FragmentShader fs, fsWater;
      } displaceData;

    struct Water{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;

      MyGL::Texture2d      waterHeightMap[2], envMap;
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
                   const MyGL::Texture2d &smCl,
                   const Scene &scene,
                   const MyGL::AbstractCamera &camera);

    void renderVolumeLight( const Scene &scene,
                            MyGL::Texture2d &gbuffer,
                            MyGL::Texture2d &mainDepth,
                            MyGL::Texture2d &shadowMap );

    void drawOmni( MyGL::Texture2d *gbuffer,
                   MyGL::Texture2d &mainDepth,
                   MyGL::Texture2d &sm,
                   const Scene &scene);

    void setupLight( const Scene &scene,
                     MyGL::FragmentShader & fs,
                     const MyGL::Texture2d &sm,
                     const MyGL::Texture2d &smCl);

    void fillShadowMap( MyGL::Texture2d &sm,
                        const Scene &scene );

    void fillTranscurentMap( MyGL::Texture2d &sm,
                             MyGL::Texture2d &depthSm,
                             const Scene &scene );

    void fillShadowMap( MyGL::Texture2d &sm,
                        MyGL::Texture2d &depth,
                        const Scene &scene,
                        const Scene::Objects &v );

    void drawObjects( MyGL::Texture2d* gbuffer,
                      MyGL::Texture2d &mainDepth,
                      int bufC,
                      const Scene &scene,
                      const MyGL::AbstractCamera &camera,
                      const Scene::Objects &v,
                      void (Material::*func)( MyGL::RenderState& /*d*/,
                                              const MyGL::Matrix4x4 & /*object*/,
                                              const MyGL::AbstractCamera&,
                                              MyGL::UniformTable &,
                                              const MyGL::Matrix4x4 & ) const,
                      bool clr );

    void drawObjects(MyGL::VertexShader &vs,
                      MyGL::FragmentShader &fs,
                      MyGL::Texture2d* gbuffer,
                      MyGL::Texture2d &mainDepth,
                      int bufC,
                      const Scene &scene,
                      const MyGL::AbstractCamera &camera,
                      const Scene::Objects &v,
                      void (Material::*func)( MyGL::RenderState& /*d*/,
                                              const MyGL::Matrix4x4 & /*object*/,
                                              const MyGL::AbstractCamera&,
                                              MyGL::UniformTable &,
                                              const MyGL::Matrix4x4 & ) const,
                      bool clr = false,
                      bool clrDepth = false);

    void drawTranscurent( MyGL::Texture2d &screen,
                          MyGL::Texture2d& mainDepth,
                          MyGL::Texture2d &sceneCopy,
                          const Scene &scene,
                          const Scene::Objects &v ) ;

    void drawWater( MyGL::Texture2d &screen,
                    MyGL::Texture2d& mainDepth,
                    MyGL::Texture2d &sceneCopy,
                    MyGL::Texture2d &sm, MyGL::Texture2d &smCl,
                    MyGL::Texture2d& sceneDepth,
                    const Scene &scene,
                    const Scene::Objects &v ) ;

    void drawGlow( MyGL::Texture2d &out,
                   MyGL::Texture2d &depth,
                   const Scene &scene,
                   int size );

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
    void drawFogOfWar( MyGL::Texture2d &out, const Scene &scene);

    void blt(const MyGL::Texture2d &tex);

    void waves( MyGL::Texture2d &out,
                const MyGL::Texture2d& in,
                const MyGL::Texture2d &in1);

    void ssao( MyGL::Texture2d &out,
               const MyGL::Texture2d& in,
               const MyGL::Texture2d &gao,
               const Scene &scene);

    void aceptSsao( const Scene &s,
                    MyGL::Texture2d &out,
                    const MyGL::Texture2d& scene,
                    const MyGL::Texture2d &diff,
                    const MyGL::Texture2d &ssao);

    void aceptGI( const Scene &s,
                  MyGL::Texture2d &out,
                  const MyGL::Texture2d& scene,
                  const MyGL::Texture2d &diff,
                  const MyGL::Texture2d &norm,
                  const MyGL::Texture2d &depth,
                  const MyGL::Texture2d gi[4] );

    void ssaoDetail( MyGL::Texture2d &out,
                     const MyGL::Texture2d& in,
                     const MyGL::Texture2d &macro );

    void ssaoGMap( const Scene &s,
                   MyGL::Texture2d &out );

    MyGL::Texture2d colorBuf( int w, int h );
    MyGL::Texture2d depth( int w, int h );
    MyGL::Texture2d depth( const MyGL::Size& sz );

    void renderScene( const Scene &scene,
                      const MyGL::AbstractCamera &camera,
                      MyGL::Texture2d gbuffer[4],
                      MyGL::Texture2d & depthBuffer,
                      MyGL::Texture2d rsm[],
                      int shadowMapSize, bool useAO);
    void buildRSM( Scene &scene,
                   MyGL::Texture2d gbuffer[4],
                   int shadowMapSize);

    size_t time;

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

    template< class ... Args, class ... FArgs >
    void draw( MyGL::Render & render,
               const Frustum &frustum,
               const Scene & scene,
               const MyGL::AbstractCamera & camera,
               const Scene::Objects & obj,
               void (Material::*func)( MyGL::RenderState& /*d*/,
                                       const MyGL::Matrix4x4 & /*object*/,
                                       const MyGL::AbstractCamera&,
                                       MyGL::UniformTable &,
                                       FArgs ... args ) const,
               Args... args );
  };


#endif // GRAPHICSSYSTEM_H
