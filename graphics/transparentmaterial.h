#ifndef TRANSPARENTMATERIAL_H
#define TRANSPARENTMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Texture2d>
#include <MyGL/Scene>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

class GraphicsSystem;

class TransparentMaterial : public MyGL::AbstractMaterial {
  public:
    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;

    MyGL::Texture2d texture;
};

class TransparentMaterialZPass : public TransparentMaterial {
  public:
    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;
};

#endif // TRANSPARENTMATERIAL_H
