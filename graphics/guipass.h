#ifndef GUIPASS_H
#define GUIPASS_H

#include <MyGL/AbstractRenderPass>

#include <MyGL/VertexShaderHolder>
#include <MyGL/FragmentShaderHolder>

#include <MyGL/Size>
#include <MyWidget/Painter>
#include <MyGL/RenderState>

#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include "pixmapspool.h"

#include <cstdint>

namespace MyGL{
  class PostProcessHelper;
  }

class MainGui;
class Resource;

class GUIPass : public MyGL::AbstractRenderPass {
  public:
    GUIPass( const MyGL::VertexShader   & vsh,
             const MyGL::FragmentShader & fsh,
             MyGL::VertexBufferHolder &vbo,
             MyGL::Size &s );

    void exec( MainGui &gui, MyGL::Texture2d &rt,
               MyGL::Texture2d &depth, MyGL::Device &device );

    void rect( int x0, int y0, int x1, int y1,
               int texDx, int texDy, int tw, int th );

    void setTexture( const PixmapsPool::TexturePtr &t );
    void unsetTexture();

    void clearBuffers();
    void setBlendMode( MyWidget::BlendMode m );

    void setCurrentBuffer( int i );

    void setColor( float r, float g, float b, float a );

    void pushState();
    void popState();
  private:
    struct Vertex{
      float x,y;
      float u,v;
      float color[4];
      };
    MyWidget::Rect texRect;

    struct GeometryBlock{
      size_t begin, size;
      MyGL::RenderState state;
      PixmapsPool::TexturePtr texture;
      };

    struct Layer{
      std::vector<GeometryBlock> geometryBlocks;
      MyGL::VertexBuffer<Vertex> guiGeometry;

      std::vector<Vertex> guiRawData;
      bool needToUpdate;
      };

    std::vector<Layer> layers;
    int curLay;

    MyGL::VertexDeclaration    vdecl;

    MyGL::VertexShader   vs;
    MyGL::FragmentShader fs;
    MyGL::VertexBufferHolder & vbHolder;

    MyGL::Texture2d noTexture, testTex;

    MyGL::Size         & size;
    MyWidget::Size       texSize;

    float dTexCoord[2];

    struct State{
      MyGL::Texture2d tex;
      float color[4];
      } state;

    std::vector<State> stateStk;

    MyGL::Device *dev;

    static MyGL::RenderState makeRS( MyWidget::BlendMode m );
  };

#endif // GUIPASS_H
