#ifndef DISPLACEMATERIAL_H
#define DISPLACEMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Matrix4x4>
#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include <MyGL/Scene>

namespace MyGL{
  class Scene;
  class RenderTaget;
  }

class GraphicsSystem;

class DisplaceMaterial : public MyGL::AbstractMaterial {
  public:
    DisplaceMaterial( const MyGL::Matrix4x4 & shadowMatrix  );

    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable & table) const;

    bool  useAlphaTest;
    float alphaTrestRef;

    MyGL::Texture2d normalMap;
  protected:
    const MyGL::Matrix4x4 * shadowMatrix;
  };

#endif // DISPLACEMATERIAL_H
