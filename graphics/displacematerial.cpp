#include "displacematerial.h"

#include <MyGL/AbstractCamera>
#include <MyGL/UniformTable>
#include <MyGL/RenderState>

#include "graphics/graphicssystem.h"

DisplaceMaterial::DisplaceMaterial(const MyGL::Matrix4x4 &mat)
  :shadowMatrix(&mat) {

  }

bool DisplaceMaterial::bind( MyGL::RenderState &rs,
                             const MyGL::Matrix4x4 &object,
                             const MyGL::AbstractCamera &c,
                             MyGL::UniformTable & table ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);
  rs.setZWriting( true );

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  MyGL::Matrix4x4 vp = c.projective();
  vp.mul( c.view() );

  MyGL::Matrix4x4 sh = *shadowMatrix;
  sh.mul( object );

  table.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex );
  table.add( vp,      "mvpMatrix",    MyGL::UniformTable::Fragment );

  table.add( object,  "objectMatrix", MyGL::UniformTable::Vertex );

  table.add( normalMap,  "normalMap", MyGL::UniformTable::Fragment );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }

  return true;
  }

