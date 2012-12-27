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
    TransparentMaterial( const MyGL::Matrix4x4 & shadowMatrix );

    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;

    MyGL::Texture2d texture;

  private:
    const MyGL::Matrix4x4 * shadowMatrix;
};

class TransparentMaterialNoZW : public TransparentMaterial {
  public:
    TransparentMaterialNoZW(const MyGL::Matrix4x4 & s );

    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;
};

class TransparentMaterialZPass : public MyGL::AbstractMaterial {
  public:
    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;
    MyGL::Texture2d texture;
};

#endif // TRANSPARENTMATERIAL_H
