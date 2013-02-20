#include "grassmaterial.h"

#include <MyGL/UniformTable>
#include <MyGL/RenderState>

#include <MyGL/Camera>

#include "blushmaterial.h"

GrassMaterial::GrassMaterial( const MyGL::Matrix4x4 &s  )
             : shadowMatrix(&s) {
  useAlphaTest  = false;
  alphaTrestRef = 0.1;
  }

bool GrassMaterial::bind( MyGL::RenderState &rs,
                          const MyGL::Matrix4x4 &object,
                          const MyGL::AbstractCamera &c,
                          MyGL::UniformTable & table ) const {
  MyGL::Matrix4x4 mobj = BlushMaterial::animateObjMatrix(object, 8);
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( mobj );

  MyGL::Matrix4x4 sh = *shadowMatrix;
  sh.mul( mobj );

  table.add( m,      "mvpMatrix",    MyGL::UniformTable::Vertex );
  table.add( mobj,   "objectMatrix", MyGL::UniformTable::Vertex );
  table.add( sh,     "shadowMatrix", MyGL::UniformTable::Vertex );

  table.add( diffuseTexture, "texture",   MyGL::UniformTable::Fragment );

  rs.setBlend(1);
  rs.setZWriting(0);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                   MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }

  return true;
  }

