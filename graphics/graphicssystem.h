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

class GraphicsSystem {
  public:
    GraphicsSystem( void *hwnd, int w, int h, bool isFullScreen, int smSize );

    void render( const MyGL::Scene &scene, MyGL::Camera &camera );
    void resizeEvent( int w, int h, bool isFullScreen );

    void load( Resource & r, MainGui & gui, int w, int h );

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
    static  MyGL::Matrix4x4 makeShadowMatrix( const MyGL::Scene & s );

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

      MyGL::Uniform<float[3]> lightDirection, lightColor, lightAblimient,
                              view;
      } gbuf;

    struct Transparent{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } transparentData;

    struct Displace{
      MyGL::VertexShader   vs, vsWater;
      MyGL::FragmentShader fs, fsWater;
      } displaceData;

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

    struct Final{
      MyGL::Uniform< MyGL::Texture2d > scene, bloom, glow;
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } finalData;

    MyGL::Uniform<float[2]> scrOffset, cpyOffset;

    void fillGBuf( MyGL::Texture2d *gbuffer,
                   MyGL::Texture2d &mainDepth,
                   const MyGL::Texture2d &sm, const MyGL::Scene &scene);

    void fillShadowMap( MyGL::Texture2d &sm,
                        const MyGL::Scene &scene );

    void drawObjects( MyGL::Texture2d* gbuffer,
                      MyGL::Texture2d &mainDepth,
                      const MyGL::Scene &scene, const MyGL::Scene::Objects &v, bool clr );

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
                          const MyGL::Scene &scene, const MyGL::Scene::Objects &v ) ;

    void drawGlow( MyGL::Texture2d &out,
                   MyGL::Texture2d &depth, const MyGL::Scene &scene );

    void copy( MyGL::Texture2d &out,
               const MyGL::Texture2d& in );
    void copy( MyGL::Texture2d &out,
               const MyGL::Texture2d& in, int w, int h );
    void gauss( MyGL::Texture2d &out,
                const MyGL::Texture2d& in, int w, int h, float dx, float dy );

    void bloom( MyGL::Texture2d &out,
                const MyGL::Texture2d& in );
    void blt(const MyGL::Texture2d &tex);

    MyGL::Texture2d depth( int w, int h );
    MyGL::Texture2d depth( const MyGL::Size& sz );

    friend class DisplaceMaterial;
    friend class GlowMaterial;
    friend class TransparentMaterial;
    friend class WaterMaterial;
  };


#endif // GRAPHICSSYSTEM_H
