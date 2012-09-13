#include "mainmaterial.h"

#include <MyGL/UniformTable>
#include <MyGL/RenderState>

MainMaterial::MainMaterial( const MyGL::Texture2d &sm,
                            const MyGL::Matrix4x4 &s, const MyGL::Color &teamColor )
             : shadowMap(&sm), shadowMatrix(&s), teamColor(&teamColor) {
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
  table.add( *shadowMap,     "shadowMap", MyGL::UniformTable::Fragment );

  table.add( specular, "specularFactor", MyGL::UniformTable::Fragment );

  float cl[3] = { teamColor->r()/255.0f,
                  teamColor->g()/255.0f,
                  teamColor->b()/255.0f };
  table.add( cl, 3, "tmColor", MyGL::UniformTable::Fragment );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::Always );
    rs.setAlphaTestRef( alphaTrestRef );
    }

  return true;
  }
