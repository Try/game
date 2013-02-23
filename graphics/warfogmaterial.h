#ifndef WARFOGMATERIAL_H
#define WARFOGMATERIAL_H

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

class WarFogMaterial  : public MyGL::AbstractMaterial {
  public:
    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable & table) const;

    MyGL::Texture2d texture;

  };

class WarFogMaterialZPass  : public MyGL::AbstractMaterial {
  public:
    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable & table) const;

  };

#endif // WARFOGMATERIAL_H
