#include "omnimaterial.h"

#include <MyGL/UniformTable>
#include <MyGL/RenderState>

bool OmniMaterial::bind( MyGL::RenderState &rs,
                         const MyGL::Matrix4x4 &object,
                         const MyGL::AbstractCamera &c,
                         MyGL::UniformTable & u) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix", MyGL::UniformTable::Vertex   );

  rs.setZWriting(0);
  rs.setBlend(1);
  //rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::Greater );
  rs.setAlphaTestRef(0.01);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::one,
                   MyGL::RenderState::AlphaBlendMode::one );

  return 1;
  }
