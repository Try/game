#include "glowmaterial.h"

#include <MyGL/AbstractCamera>
#include <MyGL/UniformTable>
#include <MyGL/RenderState>

#include "graphics/graphicssystem.h"

GlowMaterial::GlowMaterial() {
  }

bool GlowMaterial::bind( MyGL::RenderState &rs,
                         const MyGL::Matrix4x4 &object,
                         const MyGL::AbstractCamera &c,
                         MyGL::UniformTable & table ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  table.add( m,      "mvpMatrix",    MyGL::UniformTable::Vertex   );
  table.add( texture,  "texture",    MyGL::UniformTable::Fragment );

  return true;
  }
