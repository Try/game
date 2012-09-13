#ifndef ADDMATERIAL_H
#define ADDMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Texture2d>
#include <MyGL/Scene>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

class GraphicsSystem;

class AddMaterial : public MyGL::AbstractMaterial {
  public:
    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;

    MyGL::Texture2d texture;
  };

#endif // ADDMATERIAL_H
