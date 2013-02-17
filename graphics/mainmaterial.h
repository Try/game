#ifndef MAINMATERIAL_H
#define MAINMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include <MyGL/Color>

class MainMaterial : public MyGL::AbstractMaterial {
  public:
    MainMaterial( const MyGL::Matrix4x4 & shadowMatrix,
                  const MyGL::Color & teamColor );
    MainMaterial( const MyGL::Matrix4x4 & shadowMatrix );

    MyGL::Texture2d diffuseTexture,
                    normalMap;
    bool  useAlphaTest;
    float alphaTrestRef, specular;

  protected:
    const MyGL::Matrix4x4 * shadowMatrix;
    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable &  ) const ;

  private:
    const MyGL::Color * teamColor;
};

#endif // MAINMATERIAL_H
