#ifndef GRAPHICSSYSTEM_H
#define GRAPHICSSYSTEM_H

#include <stdint.h>

#include <MyGL/DirectX9>
#include <MyGL/Device>

#include <MyGL/VertexBufferHolder>

#include <MyGL/IndexBufferHolder>

#include <MyGL/TextureHolder>

#include <MyGL/VertexShaderHolder>
#include <MyGL/FragmentShaderHolder>

#include <MyGL/GraphicObject>
#include <MyGL/Scene>

#include <MyGL/Algo/GBufferFillPass>
#include <MyGL/Algo/RenderAlgo>
#include <MyGL/Algo/ShadowMapPass>

#include <MyGL/Size>
#include <MyGL/Algo/PostProcessHelper>

#include "graphics/mainpass.h"

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
    MyGL::VertexBufferHolder   vboHolder;
    MyGL::IndexBufferHolder    iboHolder;

    MyGL::VertexShaderHolder   vsHolder;
    MyGL::FragmentShaderHolder fsHolder;

    MyGL::PostProcessHelper    ppHelper;


    struct ObjectsClosure{
      const MyGL::ShadowMapPassBase::ShadowMap * shadowMap;

      struct{
        MyGL::Matrix4x4 matrix;
        } shadow;

      } closure;
  private:
    MainPass::Buffer  gbuffer;
    MyGL::RenderTaget              sceneCopy;
    MyGL::Texture2d                sceneCopyDepth, mainDepth;

    MyGL::ShadowMapPassBase::ShadowMap shadowMap;
    MyGL::Texture2d                    depthSmBuffer;
    MyGL::RenderTaget                  finalImage;

    MyGL::RenderTaget                  bpRt, dwRt[3], gaussTmp[3],
                                       bloom[3], glow, glowTmp;
    MyGL::Texture2d                    dwRtDS, glowDS;

    MyGL::RenderTaget waterNormals;
    MyGL::Texture2d   wDepth, *waterHeightMap;

    MyGL::RenderAlgo           renderAlgo;
    MyGL::RenderAlgo           makeRenderAlgo( Resource &res, MainGui &gui, int w, int h );
    MyGL::RenderAlgo           makeGlowAlgo( Resource &res, MainGui &gui, int w, int h );
    MyGL::RenderAlgo           makeBloomAlgo( Resource &res, MainGui &gui, int w, int h );

    MyGL::Size  screenSize;

    void makeBuffers(int w, int h);

    typedef MyGL::Matrix4x4 (*MakeShadowMatrix)( const MyGL::Scene & s );
    static  MyGL::Matrix4x4 makeShadowMatrix( const MyGL::Scene & s );

    static void copyPassBind ( const MyGL::Scene &,
                               MyGL::Device &device,
                               const MyGL::RenderTaget &t,
                               MyGL::VertexShader      &  vs,
                               MyGL::FragmentShader    &  fs );

    static void waterPassBind ( const MyGL::Scene &,
                                MyGL::Device &device,
                                const MyGL::Texture2d &t,
                                MyGL::VertexShader      &  vs,
                                MyGL::FragmentShader    &  fs );

    static void downSamplerBind( const MyGL::Scene &s,
                                 MyGL::Device &d,
                                 const MyGL::RenderTaget &input,
                                 MyGL::VertexShader      &  vs,
                                 MyGL::FragmentShader    &  fs );

    static void brightPassBind ( const MyGL::Scene &,
                                 MyGL::Device &device,
                                 const GraphicsSystem & sys,
                                 MyGL::VertexShader      &  vs,
                                 MyGL::FragmentShader    &  fs );

    static void gaussBindH( const MyGL::Scene &s,
                            MyGL::Device &d,
                            const MyGL::RenderTaget &input,
                            MyGL::VertexShader      &  vs,
                            MyGL::FragmentShader    &  fs );

    static void gaussBindV( const MyGL::Scene &s,
                            MyGL::Device &d,
                            const MyGL::RenderTaget &input,
                            MyGL::VertexShader      &  vs,
                            MyGL::FragmentShader    &  fs );

    static void bloomeCombineBind ( const MyGL::Scene &,
                                    MyGL::Device &device,
                                    const GraphicsSystem & sys,
                                    MyGL::VertexShader      &  vs,
                                    MyGL::FragmentShader    &  fs );

    static void finalPassBind ( const MyGL::Scene &,
                                MyGL::Device &device,
                                const GraphicsSystem & sys,
                                MyGL::VertexShader      &  vs,
                                MyGL::FragmentShader    &  fs );

    struct Glow{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } glowData;

    struct Transparent{
      MyGL::VertexShader   vs;
      MyGL::FragmentShader fs;
      } transparentData;

    struct Displace{
      MyGL::VertexShader   vs, vsWater;
      MyGL::FragmentShader fs, fsWater;
      } displaceData;

    friend class DisplaceMaterial;
    friend class GlowMaterial;
    friend class TransparentMaterial;
    friend class WaterMaterial;
  };


#endif // GRAPHICSSYSTEM_H
