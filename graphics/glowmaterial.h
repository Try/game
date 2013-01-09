#ifndef GLOWMATERIAL_H
#define GLOWMATERIAL_H

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

class GlowMaterial : public MyGL::AbstractMaterial {
  public:
    GlowMaterial();

    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable & table) const;

    MyGL::Texture2d texture;

    static void exec( const MyGL::Scene & scene,
                      const MyGL::Scene::Objects &v,
                      MyGL::Device & device,
                      GraphicsSystem &rt );

  };

#endif // GLOWMATERIAL_H
