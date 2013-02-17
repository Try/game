#include "grassmaterial.h"

#include <MyGL/UniformTable>
#include <MyGL/RenderState>

#include <MyGL/Camera>

GrassMaterial::GrassMaterial( const MyGL::Matrix4x4 &s  )
             : shadowMatrix(&s) {
  useAlphaTest  = false;
  alphaTrestRef = 0.1;
  }

bool GrassMaterial::bind( MyGL::RenderState &rs,
                          const MyGL::Matrix4x4 &object,
                          const MyGL::AbstractCamera &c,
                          MyGL::UniformTable & table ) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  MyGL::Matrix4x4 sh = *shadowMatrix;
  sh.mul( object );

  table.add( m,      "mvpMatrix",    MyGL::UniformTable::Vertex );
  table.add( object, "objectMatrix", MyGL::UniformTable::Vertex );
  table.add( sh,     "shadowMatrix", MyGL::UniformTable::Vertex );

  table.add( diffuseTexture, "texture",   MyGL::UniformTable::Fragment );

  rs.setBlend(1);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                   MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }

  return true;
  }

