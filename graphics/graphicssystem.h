#ifndef GRAPHICSSYSTEM_H
#define GRAPHICSSYSTEM_H

#include <stdint.h>

#include <Tempest/DirectX9>
#include <Tempest/Opengl2x>
#include <Tempest/Device>

#include <Tempest/VertexBufferHolder>
#include <Tempest/LocalVertexBufferHolder>

#include <Tempest/IndexBufferHolder>

#include <Tempest/TextureHolder>
#include <Tempest/LocalTexturesHolder>

#include <Tempest/VertexShaderHolder>
#include <Tempest/FragmentShaderHolder>

#include <Tempest/GraphicObject>
#include <Tempest/AbstractScene>
#include <Tempest/PostProcessHelper>

#include "graphics/guipass.h"
#include "graphics/scene.h"
#include "gui/graphicssettingswidget.h"

#include <Tempest/signal>

namespace Tempest{
  class Camera;
  }

class Resource;
class MainGui;
class ParticleSystemEngine;

class GraphicsSystem {
  public:
    GraphicsSystem(void *hwnd, bool isFullScreen);

    bool render( Scene &scene,
                 ParticleSystemEngine &e, Tempest::Camera camera,
                 size_t dt );

    void resizeEvent( int w, int h, bool isFullScreen );

    void load( Resource & r, MainGui & gui, int w, int h );

    void renderSubScene( const Scene &scene,
                         ParticleSystemEngine &e,
                         Tempest::Texture2d & out   );

    Tempest::signal<double> onRender;
    void setFog( const Tempest::Pixmap& p );

    void setSettings( const GraphicsSettingsWidget::Settings& s );

  private:
    std::shared_ptr<Tempest::AbstractAPI> api;

    static Tempest::Device::Options makeOpt( bool isFullScreen );
  public:
    Tempest::Device   device;

    Tempest::TextureHolder           texHolder;
    Tempest::LocalTexturesHolder     localTex;
    Tempest::VertexBufferHolder      vboHolder;
    Tempest::LocalVertexBufferHolder lvboHolder;
    Tempest::IndexBufferHolder       iboHolder;
    Tempest::LocalIndexBufferHolder  liboHolder;

    Tempest::VertexShaderHolder   vsHolder;
    Tempest::FragmentShaderHolder fsHolder;

    Tempest::PostProcessHelper    ppHelper;
    Tempest::Texture2d    fogView;
    Resource * resource;


    struct ObjectsClosure{
      struct{
        Tempest::Matrix4x4 matrix;
        } shadow;

      } closure;

    struct Frustum{
      float f[6][4];
      };

    static void mkFrustum( const Tempest::AbstractCamera& c, Frustum& out );
    static void mkFrustum( const Tempest::Matrix4x4& c, Frustum& out );

    enum VisibleRet{
      NotVisible = 0,
      PartialVisible,
      FullVisible
      };

    static VisibleRet isVisible( const AbstractGraphicObject& c, const Frustum& f );
    static VisibleRet isVisible( float x, float y, float z, float r, const Frustum& f );

    static int dipOptimizeRef();
  private:
    GraphicsSettingsWidget::Settings settings;
    std::unique_ptr<GUIPass> gui;
    MainGui * widget;
    ParticleSystemEngine * particles;

    bool useFog, useHDR, useDirectRender;

    Tempest::Size  guiSize, screenSize, potScreenSize;
    size_t nFrame;

    static  float smMatSize(const Scene & s, float sv = 0 );
    static  Tempest::Matrix4x4 makeShadowMatrix( const Scene & s );
    static  Tempest::Matrix4x4 makeShadowMatrix(const Scene & s, float *dxyz,
                                                 float sv = 0 );

    Tempest::Texture2d::Sampler reflect, bufSampler;

    void makeRenderAlgo(int w, int h );
    void blurSm( Tempest::Texture2d &sm,
                 Tempest::Texture2d &out,
                 const Scene &scene );

    struct Sm{
      Tempest::VertexShader   vs;
      Tempest::FragmentShader fs;
      } smap;

    struct GBuf{
      Tempest::VertexShader   vs, grassVs;
      Tempest::FragmentShader fs, grassFs;

      Tempest::VertexShader   terrainVs;
      Tempest::FragmentShader terrainFs;

      Tempest::Uniform<float[3]> lightDirection, lightColor, lightAblimient,
                              view;
      } gbuf;

    struct Transparent{
      Tempest::VertexShader   vs, vsAdd, vsSh;
      Tempest::FragmentShader fs, fsAdd, fsSh;
      } transparentData;

    struct Displace{
      Tempest::VertexShader   vs, vsWater;
      Tempest::FragmentShader fs, fsWater;
      } displaceData;

    struct Water{
      Tempest::VertexShader   vs;
      Tempest::FragmentShader fs;

      Tempest::Texture2d      waterHeightMap[2], envMap;
      } water;

    struct Glow{
      Tempest::VertexShader   vs;
      Tempest::FragmentShader fs;
      } glowData;

    struct FogOfWar{
      Tempest::VertexShader   vs, vsAcept;
      Tempest::FragmentShader fs, fsAcept;
      } fogOfWar;

    struct Bloom{
      Tempest::Uniform< Tempest::Texture2d > b[3];

      Tempest::VertexShader   vs;
      Tempest::FragmentShader brightPass, combine;
      } bloomData;

    struct Blt{
      Tempest::Uniform< Tempest::Texture2d > texture;
      Tempest::VertexShader   vs;
      Tempest::FragmentShader fs;
      } bltData;

    struct Gauss{
      Tempest::Uniform< Tempest::Texture2d > texture;
      Tempest::VertexShader   vs, vsAO, vsGB, vsB;
      Tempest::FragmentShader fs, fsAO, fsGB, fsB;
      } gaussData;

    struct Omni{
      Tempest::Uniform< Tempest::Texture2d > texture;
      Tempest::VertexShader   vs;
      Tempest::FragmentShader fs;
      } omniData;

    struct Volumetric{
      Tempest::Uniform< Tempest::Texture2d > scene;
      Tempest::VertexShader   vs;
      Tempest::FragmentShader fs;
      } volumetricData;

    struct Final{
      Tempest::Uniform< Tempest::Texture2d > scene, bloom, glow;
      Tempest::VertexShader   vs;
      Tempest::FragmentShader fs, avatar;
      } finalData;

    struct SSAO{
      Tempest::Uniform< Tempest::Texture2d > texture, blured, macro;
      Tempest::Uniform< Tempest::Texture2d > scene, diff, ssao;
      Tempest::Uniform<float[3]> lightAblimient;

      Tempest::VertexShader   vs;
      Tempest::FragmentShader acceptGI;
      } ssaoData;

    Tempest::Uniform<float[2]> scrOffset, cpyOffset;

    void fillGBuf(Tempest::Texture2d *gbuffer,
                   Tempest::Texture2d &mainDepth,
                   const Tempest::Texture2d &sm,
                   const Tempest::Texture2d &smCl, const Tempest::Texture2d &ao,
                   const Scene &scene,
                   const Tempest::AbstractCamera &camera);

    void renderVolumeLight( const Scene &scene,
                            Tempest::Texture2d &gbuffer,
                            Tempest::Texture2d &mainDepth,
                            Tempest::Texture2d &shadowMap );

    void drawOmni( Tempest::Texture2d *gbuffer,
                   Tempest::Texture2d &mainDepth,
                   Tempest::Texture2d &sm,
                   const Scene &scene);

    void setupLight(const Scene &scene, Tempest::VertexShader &vs,
                     Tempest::FragmentShader & fs,
                     const Tempest::Texture2d &sm,
                     const Tempest::Texture2d &smCl, const Tempest::Texture2d &ao);

    void fillShadowMap(Tempest::Texture2d &sm,
                        const Scene &scene , bool clr);

    void fillTranscurentMap(Tempest::Texture2d &sm,
                             Tempest::Texture2d &depthSm,
                             const Scene &scene , bool clr);

    void fillShadowMap(Tempest::Texture2d &sm,
                        Tempest::Texture2d &depth,
                        const Scene &scene,
                        const Scene::Objects &v , bool clr);

    int drawObjects( Tempest::Texture2d* gbuffer,
                      Tempest::Texture2d* mainDepth,
                      int bufC,
                      const Scene &scene,
                      const Tempest::AbstractCamera &camera,
                      const Scene::Objects &v,
                      void (Material::*func)( Tempest::RenderState& /*d*/,
                                              const Tempest::Matrix4x4 & /*object*/,
                                              const Tempest::AbstractCamera&,
                                              Tempest::UniformTable & ) const,
                      bool clr );

    int drawObjects(Tempest::VertexShader &vs,
                      Tempest::FragmentShader &fs,
                      Tempest::Texture2d* gbuffer,
                      Tempest::Texture2d* mainDepth,
                      int bufC,
                      const Scene &scene,
                      const Tempest::AbstractCamera &camera,
                      const Scene::Objects &v,
                      void (Material::*func)(Tempest::RenderState &,
                                             const Tempest::Matrix4x4 &,
                                             const Tempest::AbstractCamera &,
                                             Tempest::UniformTable &) const,
                      bool clr = false,
                      bool clrDepth = false);

    void drawTranscurent( Tempest::Texture2d &screen,
                          Tempest::Texture2d& mainDepth,
                          Tempest::Texture2d &sceneCopy,
                          const Scene &scene,
                          const Scene::Objects &v ) ;

    void drawWater(Tempest::Texture2d &screen,
                    Tempest::Texture2d& mainDepth,
                    Tempest::Texture2d &sceneCopy,
                    Tempest::Texture2d &sm, Tempest::Texture2d &smCl,
                    Tempest::Texture2d& sceneDepth, Tempest::Texture2d &ao,
                    const Scene &scene,
                    const Scene::Objects &v ) ;

    void drawGlow(Tempest::Texture2d &out,
                   Tempest::Texture2d &depth, Tempest::Texture2d &mrt0,
                   const Scene &scene,
                   int size , int w, int h);

    void aceptFog(Tempest::Texture2d &in_out, const Tempest::Texture2d &fog);

    void copy( Tempest::Texture2d &out,
               const Tempest::Texture2d& in );
    void copyDepth(Tempest::Texture2d &out,
                    const Tempest::Texture2d& in , int w, int h);
    void copy( Tempest::Texture2d &out,
               const Tempest::Texture2d& in, int w, int h );
    void gauss( Tempest::Texture2d &out,
                const Tempest::Texture2d& in,
                int w, int h, float dx, float dy );
    void gauss_ao( Tempest::Texture2d &out,
                   const Tempest::Texture2d& in,
                   int w, int h, float dx, float dy );
    void gauss_gb( Tempest::Texture2d &out,
                   const Tempest::Texture2d& in,
                   int w, int h, float dx, float dy );
    void gauss_b( Tempest::Texture2d &out,
                  const Tempest::Texture2d& in,
                  int w, int h, float dx, float dy );

    void bloom( Tempest::Texture2d &out,
                const Tempest::Texture2d& in );
    void drawFogOfWar( Tempest::Texture2d &out, const Scene &scene);

    void blt(const Tempest::Texture2d &tex);

    void waves( Tempest::Texture2d &out,
                const Tempest::Texture2d& in,
                const Tempest::Texture2d &in1);

    void aceptGI( const Scene &s,
                  Tempest::Texture2d &out,
                  const Tempest::Texture2d& scene,
                  const Tempest::Texture2d &diff,
                  const Tempest::Texture2d &norm,
                  const Tempest::Texture2d &depth,
                  const Tempest::Texture2d gi[4] );

    void ssaoGMap(const Scene &s,
                   Tempest::Texture2d &out , bool clr);

    Tempest::Texture2d shadowMap( int w, int h );
    Tempest::Texture2d colorBuf( int w, int h );
    Tempest::Texture2d depth( int w, int h );
    Tempest::Texture2d depth( const Tempest::Size& sz );

    void renderScene(const Scene &scene,
                      const Tempest::AbstractCamera &camera,
                      Tempest::Texture2d *gbuffer,
                      Tempest::Texture2d & depthBuffer, int gbufferSize,
                      Tempest::Texture2d *rsm,
                      int shadowMapSize, bool clrSm);
    void buildRSM( Scene &scene,
                   Tempest::Texture2d gbuffer[4],
                   int shadowMapSize);

    size_t time;

    friend class DisplaceMaterial;
    friend class GlowMaterial;
    friend class TransparentMaterial;
    friend class WaterMaterial;

    struct RSMCamera : public Tempest::AbstractCamera {
      RSMCamera(){
        p.identity();
        }

      Tempest::Matrix4x4 view() const {
        return v;
        }

      Tempest::Matrix4x4 projective() const{
        return p;
        }

      Tempest::Matrix4x4 v, p;
      };

    template< class T >
    T& ref( T& t ){
      return t;
      }

    bool isPot( int v ){
      return (v & (v-1)) == 0;
      }

    uint32_t nextPot( uint32_t v ){
      v--;
      v |= v >> 1;
      v |= v >> 2;
      v |= v >> 4;
      v |= v >> 8;
      v |= v >> 16;
      v++;

      return v;
      }

    static Tempest::AbstractAPI* createAPI();
    void setupScreenSize( int w, int h );

    template< class ... Args, class ... FArgs >
    int draw(  Tempest::Render & render,
               const Frustum &frustum,
               bool  deepVTest,
               const Tempest::AbstractCamera & camera,
               const Scene::Objects & obj,
               void (Material::*func)( Tempest::RenderState& /*d*/,
                                       const Tempest::Matrix4x4 & /*object*/,
                                       const Tempest::AbstractCamera&,
                                       Tempest::UniformTable &,
                                       FArgs ... args ) const,
               Args... args );
    std::vector<const AbstractGraphicObject*> toDraw;

    template< class ... Args, class ... FArgs >
    void completeDraw( Tempest::Render & render,
                       const Tempest::AbstractCamera & camera,
                       void (Material::*func)( Tempest::RenderState& /*d*/,
                                               const Tempest::Matrix4x4 & /*object*/,
                                               const Tempest::AbstractCamera&,
                                               Tempest::UniformTable &,
                                               FArgs ... args ) const,
                       Args... args  );
  };


#endif // GRAPHICSSYSTEM_H
