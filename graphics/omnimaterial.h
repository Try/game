#ifndef OMNIMATERIAL_H
#define OMNIMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Texture2d>
#include <MyGL/Scene>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

class OmniMaterial : public MyGL::AbstractMaterial {
  public:
    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;
};

#endif // OMNIMATERIAL_H
