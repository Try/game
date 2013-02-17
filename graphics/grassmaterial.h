#ifndef GRASSMATERIAL_H
#define GRASSMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include <MyGL/Color>

class GrassMaterial : public MyGL::AbstractMaterial {
  public:
    GrassMaterial( const MyGL::Matrix4x4 & shadowMatrix );

    MyGL::Texture2d diffuseTexture;
    bool  useAlphaTest;
    float alphaTrestRef;

  protected:
    const MyGL::Matrix4x4 * shadowMatrix;
    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable &  ) const ;
  };

#endif // GRASSMATERIAL_H
