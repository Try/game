#include "terrainminormaterial.h"

#include <MyGL/RenderState>

#include <MyGL/UniformTable>

TerrainMinorMaterial::TerrainMinorMaterial(const MyGL::Matrix4x4 &sm)
                     :MainMaterial(sm){

  }

bool TerrainMinorMaterial::bind( MyGL::RenderState &rs,
                                 const MyGL::Matrix4x4 &object,
                                 const MyGL::AbstractCamera &c,
                                 MyGL::UniformTable & u ) const {
  MainMaterial::bind( rs, object, c, u );

  rs.setZTestMode( MyGL::RenderState::ZTestMode::Equal );
  rs.setZWriting(0);
  rs.setBlend(1);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::one,
                   MyGL::RenderState::AlphaBlendMode::one );

  return 1;
  }


bool TerrainZPass::bind( MyGL::RenderState &rs,
                         const MyGL::Matrix4x4 &object,
                         const MyGL::AbstractCamera &c,
                         MyGL::UniformTable & u ) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix", MyGL::UniformTable::Vertex   );
  u.add( texture, "texture",  MyGL::UniformTable::Fragment );

  rs.setAlphaTestRef(0.01);
  rs.setColorMask(0,0,0,0);

  return 1;
  }
