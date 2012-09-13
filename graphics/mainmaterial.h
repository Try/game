#ifndef MAINMATERIAL_H
#define MAINMATERIAL_H

#include <MyGL/AbstractMaterial>
#include <MyGL/Texture2d>
#include <MyGL/Scene>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include <MyGL/Color>

class GraphicsSystem;

class MainMaterial : public MyGL::AbstractMaterial {
  public:
    MainMaterial( const MyGL::Texture2d & shadowMap,
                  const MyGL::Matrix4x4 & shadowMatrix,
                  const MyGL::Color & teamColor );

    MyGL::Texture2d diffuseTexture,
                    normalMap;
    bool  useAlphaTest;
    float alphaTrestRef, specular;

  protected:
    const MyGL::Texture2d * shadowMap;
    const MyGL::Matrix4x4 * shadowMatrix;

  private:
    bool bind( MyGL::RenderState & dev,
               const MyGL::Matrix4x4 & object,
               const MyGL::AbstractCamera & c,
               MyGL::UniformTable &  ) const ;

    const MyGL::Color * teamColor;
};

#endif // MAINMATERIAL_H
