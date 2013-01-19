#include "watermaterial.h"

#include <MyGL/AbstractCamera>
#include <MyGL/UniformTable>
#include <MyGL/RenderState>

#include <cmath>

#include "graphics/graphicssystem.h"

WaterMaterial::WaterMaterial( const MyGL::Matrix4x4 &mat)
  : shadowMatrix(&mat) {

  }

bool WaterMaterial::bind( MyGL::RenderState &rs,
                          const MyGL::Matrix4x4 &object,
                          const MyGL::AbstractCamera &c,
                          MyGL::UniformTable & table ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);
  rs.setZWriting( true );

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  MyGL::Matrix4x4 sh = *shadowMatrix;
  sh.mul( object );

  table.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex   );
  table.add( object,  "objectMatrix", MyGL::UniformTable::Vertex   );
  table.add( sh,     "shadowMatrix",  MyGL::UniformTable::Vertex );

  table.add( texture, "texture",      MyGL::UniformTable::Fragment );
  //table.add( normals, "normalMap",    MyGL::UniformTable::Fragment );
  //table.add( *shadowMap, "shadowMap", MyGL::UniformTable::Fragment );
  table.add( 1, "specularFactor", MyGL::UniformTable::Fragment );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }

  return true;
  }
