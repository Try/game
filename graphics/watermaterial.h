#ifndef WATERMATERIAL_H
#define WATERMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Matrix4x4>
#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

namespace MyGL{
  class Scene;
  class RenderTaget;
  }

class GraphicsSystem;

class WaterMaterial : public MyGL::AbstractMaterial {
  public:
    WaterMaterial( const MyGL::Matrix4x4 & shadowMatrix  );

    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable & table) const;

    bool  useAlphaTest;
    float alphaTrestRef;

    MyGL::Texture2d texture, normals;

  protected:
    const MyGL::Matrix4x4 * shadowMatrix;
  };

#endif // WATERMATERIAL_H
