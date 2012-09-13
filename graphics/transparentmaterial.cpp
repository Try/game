#include "transparentmaterial.h"

#include <MyGL/UniformTable>
#include <MyGL/RenderState>

bool TransparentMaterial::bind( MyGL::RenderState &rs,
                                const MyGL::Matrix4x4 &object,
                                const MyGL::AbstractCamera &c,
                                MyGL::UniformTable & u) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix", MyGL::UniformTable::Vertex   );
  u.add( texture, "texture",  MyGL::UniformTable::Fragment );

  rs.setBlend(1);
  rs.setAlphaTestRef(0.01);
  rs.setZTestMode( MyGL::RenderState::ZTestMode::Equal );
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                   MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );

  return 1;
  }

bool TransparentMaterialZPass::bind( MyGL::RenderState &rs,
                                const MyGL::Matrix4x4 &object,
                                const MyGL::AbstractCamera &c,
                                MyGL::UniformTable & u) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix", MyGL::UniformTable::Vertex   );
  u.add( texture, "texture",  MyGL::UniformTable::Fragment );

  rs.setAlphaTestRef(0.01);
  //rs.setColorMask(0,0,0,0);

  return 1;
  }
