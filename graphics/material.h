#ifndef MATERIAL_H
#define MATERIAL_H

#include <Tempest/Texture2d>
#include <Tempest/GraphicObject>

#include "gui/graphicssettingswidget.h"

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
    static Tempest::Color nullColor;

    struct Usage{
      bool mainPass, shadowCast, terrainMinor, terrainMain,
           atest,
           displace, water, blush, add,
           transparent, fogOfWar,
           omni, grass;
      } usage;

    void gbuffer( Tempest::RenderState& /*d*/,
                  const Tempest::Matrix4x4 & /*object*/,
                  const Tempest::AbstractCamera&,
                  Tempest::UniformTable &) const;

    void grass( Tempest::RenderState& /*d*/,
                const Tempest::Matrix4x4 & /*object*/,
                const Tempest::AbstractCamera&,
                Tempest::UniformTable &) const;

    void additive( Tempest::RenderState& /*d*/,
                   const Tempest::Matrix4x4 & /*object*/,
                   const Tempest::AbstractCamera&,
                   Tempest::UniformTable &) const;

    void terrainMain( Tempest::RenderState& /*d*/,
                      const Tempest::Matrix4x4 & /*object*/,
                      const Tempest::AbstractCamera&,
                      Tempest::UniformTable &) const;

    void terrainMinor( Tempest::RenderState& /*d*/,
                       const Tempest::Matrix4x4 & /*object*/,
                       const Tempest::AbstractCamera&,
                       Tempest::UniformTable &) const;

    void terrainMinorZ( Tempest::RenderState& /*d*/,
                        const Tempest::Matrix4x4 & /*object*/,
                        const Tempest::AbstractCamera&,
                        Tempest::UniformTable &) const;

    void transparent( Tempest::RenderState& /*d*/,
                      const Tempest::Matrix4x4 & /*object*/,
                      const Tempest::AbstractCamera&,
                      Tempest::UniformTable & ) const;

    void transparentZ(Tempest::RenderState& /*d*/,
                      const Tempest::Matrix4x4 & /*object*/,
                      const Tempest::AbstractCamera&,
                      Tempest::UniformTable &) const;

    void glowPass(Tempest::RenderState& /*d*/,
                   const Tempest::Matrix4x4 & /*object*/,
                   const Tempest::AbstractCamera&,
                   Tempest::UniformTable &) const;

    void glowPassAdd(Tempest::RenderState& /*d*/,
                     const Tempest::Matrix4x4 & /*object*/,
                     const Tempest::AbstractCamera&,
                     Tempest::UniformTable &) const;

    void shadow( Tempest::RenderState& /*d*/,
                 const Tempest::Matrix4x4 & /*object*/,
                 const Tempest::AbstractCamera&,
                 Tempest::UniformTable &,
                 const Tempest::Matrix4x4 &sh ) const;

    void displace( Tempest::RenderState& /*d*/,
                   const Tempest::Matrix4x4 & /*object*/,
                   const Tempest::AbstractCamera&,
                   Tempest::UniformTable & ) const;

    void water( Tempest::RenderState& /*d*/,
                const Tempest::Matrix4x4 & /*object*/,
                const Tempest::AbstractCamera&,
                Tempest::UniformTable & ) const;

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
    static Tempest::Matrix4x4 animateObjMatrix(const Tempest::Matrix4x4 &object,
                                             float power = 1 );

    static GraphicsSettingsWidget::Settings settings;
  };

typedef Tempest::AbstractGraphicObject<Material> AbstractGraphicObject;
typedef Tempest::GraphicObject<Material>         GraphicObject;

#endif // MATERIAL_H
