#include "addmaterial.h"

#include "transparentmaterial.h"

#include <MyGL/UniformTable>
#include <MyGL/RenderState>
#include <MyGL/AbstractCamera>

bool AddMaterial::bind( MyGL::RenderState &rs,
                        const MyGL::Matrix4x4 &object,
                        const MyGL::AbstractCamera &c,
                        MyGL::UniformTable & u) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix", MyGL::UniformTable::Vertex   );
  u.add( texture, "texture",  MyGL::UniformTable::Fragment );

  rs.setBlend(1);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::one,
                   MyGL::RenderState::AlphaBlendMode::one );

  rs.setZWriting(0);
  return 1;
  }
