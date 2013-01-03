#include "mainmaterial.h"

#include <MyGL/UniformTable>
#include <MyGL/RenderState>

MainMaterial::MainMaterial( const MyGL::Matrix4x4 &s,
                            const MyGL::Color &teamColor )
             : shadowMatrix(&s), teamColor(&teamColor) {
  useAlphaTest  = false;
  alphaTrestRef = 0.5;
  specular      = 0;
  }

MainMaterial::MainMaterial( const MyGL::Matrix4x4 &s  )
             : shadowMatrix(&s), teamColor(0) {
  useAlphaTest  = false;
  alphaTrestRef = 0.5;
  specular      = 0;
  }

bool MainMaterial::bind( MyGL::RenderState &rs,
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
  table.add( normalMap,      "normalMap", MyGL::UniformTable::Fragment );
  table.add( specular, "specularFactor", MyGL::UniformTable::Fragment );

  if( teamColor ){// FIXME
    float cl[3] = { teamColor->r(),
                    teamColor->g(),
                    teamColor->b() };
    table.add( cl, 3, "tmColor", MyGL::UniformTable::Fragment );
    } else {
    float cl[3] = {};
    table.add( cl, 3, "tmColor", MyGL::UniformTable::Fragment );
    }

  if( useAlphaTest ){
    if( teamColor )
      rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::Always ); else
      rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );

    rs.setAlphaTestRef( alphaTrestRef );
    }

  return true;
  }
