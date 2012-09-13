#ifndef DISPLACEMATERIAL_H
#define DISPLACEMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Matrix4x4>
#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include <MyGL/Algo/GBufferFillPass>

namespace MyGL{
  class Scene;
  class RenderTaget;
  }

class GraphicsSystem;

class DisplaceMaterial : public MyGL::AbstractMaterial {
  public:
    DisplaceMaterial( const MyGL::Texture2d & shadowMap,
                      const MyGL::Matrix4x4 & shadowMatrix  );

    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable & table) const;

    bool  useAlphaTest;
    float alphaTrestRef;

    static void exec( const MyGL::Scene & scene,
                      const MyGL::Scene::Objects &v,
                      MyGL::Device & device,
                      GraphicsSystem &rt );

  protected:
    const MyGL::Texture2d * shadowMap;
    const MyGL::Matrix4x4 * shadowMatrix;
  };

#endif // DISPLACEMATERIAL_H
