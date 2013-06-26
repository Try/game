#ifndef GUIPASS_H
#define GUIPASS_H

#include <Tempest/VertexShaderHolder>
#include <Tempest/FragmentShaderHolder>

#include <Tempest/Painter>
#include <Tempest/RenderState>

#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

#include <Tempest/VertexBufferHolder>
#include <Tempest/VertexBuffer>

#include <Tempest/IndexBufferHolder>
#include <Tempest/IndexBuffer>
#include <Tempest/Half>

#include "pixmapspool.h"

#include <cstdint>

namespace Tempest{
  class PostProcessHelper;
  }

class MainGui;
class Resource;

class GUIPass {
  public:
    GUIPass( const Tempest::VertexShader   & vsh,
             const Tempest::FragmentShader & fsh,
             Tempest::VertexBufferHolder &vbo,
             Tempest::IndexBufferHolder  &ibo,
             Tempest::Size &s );

    void update( MainGui &gui,
                 Tempest::Device &device );
    void exec( MainGui &gui,
               Tempest::Texture2d *rt,
               Tempest::Texture2d *depth,
               Tempest::Device &device );

    void rect( int x0, int y0, int x1, int y1,
               int texDx, int texDy, int tw, int th );

    void setTexture( const PixmapsPool::TexturePtr &t );
    void unsetTexture();

    void clearBuffers();
    void setBlendMode( Tempest::BlendMode m );

    void setCurrentBuffer( int i );

    void setColor( float r, float g, float b, float a );

    void setNullState();
    void pushState();
    void popState();
  private:
    struct Vertex{
      float x,y;
      float u,v;
      float color[4];
      };

    struct HVertex{
      Tempest::Half x,y;
      Tempest::Half u,v;
      Tempest::Half color[4];
      };

    typedef Vertex Vert;

    Tempest::Rect texRect;

    struct GeometryBlock{
      size_t begin, size;
      Tempest::RenderState state;
      PixmapsPool::TexturePtr texture;
      };

    struct Layer{
      std::vector<GeometryBlock> geometryBlocks;
      Tempest::VertexBuffer<Vert>  guiGeometry;

      std::vector<Vert> guiRawData;
      bool needToUpdate;
      };

    std::vector<Layer> layers;
    Tempest::IndexBuffer<uint16_t> guiIndex;
    int curLay;

    Tempest::VertexDeclaration    vdecl;

    Tempest::VertexShader   vs;
    Tempest::FragmentShader fs;
    Tempest::VertexBufferHolder & vbHolder;
    Tempest::IndexBufferHolder  & ibHolder;

    Tempest::Texture2d noTexture, testTex;

    Tempest::Size         & size;
    Tempest::Size       texSize;

    float dTexCoord[2];

    struct State{
      Tempest::Texture2d tex;
      float color[4];
      Tempest::BlendMode blend;
      } state;

    std::vector<State> stateStk;

    Tempest::Device *dev;

    static Tempest::RenderState makeRS( Tempest::BlendMode m );
    std::vector<uint16_t> iboTmp;
  };

#endif // GUIPASS_H
