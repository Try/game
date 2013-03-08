#ifndef MATERIAL_H
#define MATERIAL_H

#include <Tempest/Texture2d>
#include <Tempest/GraphicObject>

namespace Tempest{
  class VertexShader;
  class FragmentShader;
  }

class Material {
  public:
    Material();

    Tempest::Texture2d diffuse, normal, glow;
    float specular;

    bool  useAlphaTest;
    bool  zWrighting;
    float alphaTrestRef;

    const Tempest::Color *teamColor;

    struct Usage{
      bool mainPass, shadowCast, terrainMinor,
           displace, water, blush, add,
           transparent, fogOfWar,
           omni, grass;
      } usage;

    void gbuffer( Tempest::RenderState& /*d*/,
                  const Tempest::Matrix4x4 & /*object*/,
                  const Tempest::AbstractCamera&,
                  Tempest::UniformTable &,
                  const Tempest::Matrix4x4 &shadowMatrix ) const;

    void grass( Tempest::RenderState& /*d*/,
                const Tempest::Matrix4x4 & /*object*/,
                const Tempest::AbstractCamera&,
                Tempest::UniformTable &,
                const Tempest::Matrix4x4 &shadowMatrix ) const;

    void additive( Tempest::RenderState& /*d*/,
                   const Tempest::Matrix4x4 & /*object*/,
                   const Tempest::AbstractCamera&,
                   Tempest::UniformTable &,
                   const Tempest::Matrix4x4 &shadowMatrix ) const;

    void terrainMinor( Tempest::RenderState& /*d*/,
                       const Tempest::Matrix4x4 & /*object*/,
                       const Tempest::AbstractCamera&,
                       Tempest::UniformTable &,
                       const Tempest::Matrix4x4 &shadowMatrix ) const;

    void terrainMinorZ( Tempest::RenderState& /*d*/,
                        const Tempest::Matrix4x4 & /*object*/,
                        const Tempest::AbstractCamera&,
                        Tempest::UniformTable &,
                        const Tempest::Matrix4x4 &shadowMatrix ) const;

    void transparent( Tempest::RenderState& /*d*/,
                      const Tempest::Matrix4x4 & /*object*/,
                      const Tempest::AbstractCamera&,
                      Tempest::UniformTable &,
                      const Tempest::Matrix4x4 &shadowMatrix ) const;

    void transparentZ( Tempest::RenderState& /*d*/,
                      const Tempest::Matrix4x4 & /*object*/,
                      const Tempest::AbstractCamera&,
                      Tempest::UniformTable &,
                      const Tempest::Matrix4x4 &shadowMatrix ) const;

    void glowPass( Tempest::RenderState& /*d*/,
                   const Tempest::Matrix4x4 & /*object*/,
                   const Tempest::AbstractCamera&,
                   Tempest::UniformTable &,
                   const Tempest::Matrix4x4 &unused ) const;

    void shadow( Tempest::RenderState& /*d*/,
                 const Tempest::Matrix4x4 & /*object*/,
                 const Tempest::AbstractCamera&,
                 Tempest::UniformTable &,
                 const Tempest::Matrix4x4 &sh ) const;

    void displace( Tempest::RenderState& /*d*/,
                   const Tempest::Matrix4x4 & /*object*/,
                   const Tempest::AbstractCamera&,
                   Tempest::UniformTable &,
                   const Tempest::Matrix4x4 &shadowMatrix ) const;

    void water( Tempest::RenderState& /*d*/,
                const Tempest::Matrix4x4 & /*object*/,
                const Tempest::AbstractCamera&,
                Tempest::UniformTable &,
                const Tempest::Matrix4x4 &shadowMatrix ) const;

    void omni(Tempest::RenderState& /*d*/,
               const Tempest::Matrix4x4 & /*object*/,
               const Tempest::AbstractCamera&,
               Tempest::UniformTable &,
               const Tempest::Matrix4x4 &shM ) const;

    void fogOfWar(Tempest::RenderState& /*d*/,
                   const Tempest::Matrix4x4 & /*object*/,
                   const Tempest::AbstractCamera&,
                   Tempest::UniformTable & table, bool zpass) const;

    static float wind;
  //private:
    static Tempest::Matrix4x4 animateObjMatrix( const Tempest::Matrix4x4 &object,
                                             double power = 1 );
  };

typedef Tempest::AbstractGraphicObject<Material> AbstractGraphicObject;
typedef Tempest::GraphicObject<Material>         GraphicObject;

#endif // MATERIAL_H
