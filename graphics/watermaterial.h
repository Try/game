#ifndef WATERMATERIAL_H
#define WATERMATERIAL_H

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

class WaterMaterial : public MyGL::AbstractMaterial {
  public:
    WaterMaterial( const MyGL::Texture2d & shadowMap,
                   const MyGL::Matrix4x4 & shadowMatrix  );

    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable & table) const;

    bool  useAlphaTest;
    float alphaTrestRef;

    MyGL::Texture2d texture, normals;

    static void exec( const MyGL::Scene & scene,
                      const MyGL::Scene::Objects &v,
                      MyGL::Device & device,
                      GraphicsSystem &rt );

  protected:
    const MyGL::Texture2d * shadowMap;
    const MyGL::Matrix4x4 * shadowMatrix;
  };

#endif // WATERMATERIAL_H
