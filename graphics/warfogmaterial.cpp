#include "warfogmaterial.h"

#include <MyGL/AbstractCamera>
#include <MyGL/UniformTable>
#include <MyGL/RenderState>

#include "graphics/graphicssystem.h"

bool WarFogMaterial::bind( MyGL::RenderState &rs,
                           const MyGL::Matrix4x4 &object,
                           const MyGL::AbstractCamera &c,
                           MyGL::UniformTable & table ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::Equal );
  rs.setZTest( true );
  rs.setZWriting( false );

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  table.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex   );
  table.add( object,  "objectMatrix", MyGL::UniformTable::Vertex   );
  //table.add( sh,     "shadowMatrix",  MyGL::UniformTable::Vertex );

  //table.add( texture, "texture",      MyGL::UniformTable::Fragment );

  rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
  rs.setAlphaTestRef ( 0.01 );

  //rs.setBlend(1);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                   MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );

  return true;
  }

bool WarFogMaterialZPass::bind( MyGL::RenderState &rs,
                           const MyGL::Matrix4x4 &object,
                           const MyGL::AbstractCamera &c,
                           MyGL::UniformTable & table ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setZTest( true );
  rs.setZWriting( true );

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  table.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex   );
  table.add( object,  "objectMatrix", MyGL::UniformTable::Vertex   );

  rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::Greater );
  rs.setAlphaTestRef ( 0.01 );

  rs.setColorMask(0,0,0,0);

  return true;
  }
