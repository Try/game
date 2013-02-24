#ifndef MATERIAL_H
#define MATERIAL_H

#include <MyGL/Texture2d>
#include <MyGL/GraphicObject>

namespace MyGL{
  class VertexShader;
  class FragmentShader;
  }

class Material {
  public:
    Material();

    MyGL::Texture2d diffuse, normal, glow;
    float specular;

    bool  useAlphaTest;
    bool  zWrighting;
    float alphaTrestRef;

    const MyGL::Color *teamColor;

    struct Usage{
      bool mainPass, shadowCast, terrainMinor,
           displace, water, blush, add,
           transparent, fogOfWar,
           omni, grass;
      } usage;

    void gbuffer( MyGL::RenderState& /*d*/,
                  const MyGL::Matrix4x4 & /*object*/,
                  const MyGL::AbstractCamera&,
                  MyGL::UniformTable &,
                  const MyGL::Matrix4x4 &shadowMatrix ) const;

    void grass( MyGL::RenderState& /*d*/,
                const MyGL::Matrix4x4 & /*object*/,
                const MyGL::AbstractCamera&,
                MyGL::UniformTable &,
                const MyGL::Matrix4x4 &shadowMatrix ) const;

    void additive( MyGL::RenderState& /*d*/,
                  const MyGL::Matrix4x4 & /*object*/,
                  const MyGL::AbstractCamera&,
                  MyGL::UniformTable &,
                  const MyGL::Matrix4x4 &shadowMatrix ) const;

    void terrainMinor( MyGL::RenderState& /*d*/,
                       const MyGL::Matrix4x4 & /*object*/,
                       const MyGL::AbstractCamera&,
                       MyGL::UniformTable &,
                       const MyGL::Matrix4x4 &shadowMatrix ) const;

    void terrainMinorZ( MyGL::RenderState& /*d*/,
                        const MyGL::Matrix4x4 & /*object*/,
                        const MyGL::AbstractCamera&,
                        MyGL::UniformTable &,
                        const MyGL::Matrix4x4 &shadowMatrix ) const;

    void transparent( MyGL::RenderState& /*d*/,
                      const MyGL::Matrix4x4 & /*object*/,
                      const MyGL::AbstractCamera&,
                      MyGL::UniformTable &,
                      const MyGL::Matrix4x4 &shadowMatrix ) const;

    void transparentZ( MyGL::RenderState& /*d*/,
                      const MyGL::Matrix4x4 & /*object*/,
                      const MyGL::AbstractCamera&,
                      MyGL::UniformTable &,
                      const MyGL::Matrix4x4 &shadowMatrix ) const;

    void glowPass( MyGL::RenderState& /*d*/,
                   const MyGL::Matrix4x4 & /*object*/,
                   const MyGL::AbstractCamera&,
                   MyGL::UniformTable & ) const;

    void shadow( MyGL::RenderState& /*d*/,
                 const MyGL::Matrix4x4 & /*object*/,
                 const MyGL::AbstractCamera&,
                 MyGL::UniformTable & ) const;

    void displace( MyGL::RenderState& /*d*/,
                   const MyGL::Matrix4x4 & /*object*/,
                   const MyGL::AbstractCamera&,
                   MyGL::UniformTable &,
                   const MyGL::Matrix4x4 &shadowMatrix ) const;

    void water( MyGL::RenderState& /*d*/,
                const MyGL::Matrix4x4 & /*object*/,
                const MyGL::AbstractCamera&,
                MyGL::UniformTable &,
                const MyGL::Matrix4x4 &shadowMatrix ) const;

    void omni(MyGL::RenderState& /*d*/,
               const MyGL::Matrix4x4 & /*object*/,
               const MyGL::AbstractCamera&,
               MyGL::UniformTable &,
               const MyGL::Matrix4x4 &shadowMatrix , const MyGL::Matrix4x4 &invMat) const;

    void fogOfWar(MyGL::RenderState& /*d*/,
                   const MyGL::Matrix4x4 & /*object*/,
                   const MyGL::AbstractCamera&,
                   MyGL::UniformTable & table, bool zpass) const;

    static float wind;
  //private:
    static MyGL::Matrix4x4 animateObjMatrix( const MyGL::Matrix4x4 &object,
                                             double power = 1 );
  };

typedef MyGL::AbstractGraphicObject<Material> AbstractGraphicObject;
typedef MyGL::GraphicObject<Material>         GraphicObject;

#endif // MATERIAL_H
