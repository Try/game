#include "glowmaterial.h"

#include <MyGL/AbstractCamera>
#include <MyGL/UniformTable>
#include <MyGL/RenderState>

#include "graphics/graphicssystem.h"

GlowMaterial::GlowMaterial() {
  }

bool GlowMaterial::bind( MyGL::RenderState &rs,
                         const MyGL::Matrix4x4 &object,
                         const MyGL::AbstractCamera &c,
                         MyGL::UniformTable & table ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  table.add( m,      "mvpMatrix",    MyGL::UniformTable::Vertex   );
  table.add( texture,  "texture",    MyGL::UniformTable::Fragment );

  return true;
  }

void GlowMaterial::exec( const MyGL::Scene &scene,
                         const MyGL::Scene::Objects &v,
                         MyGL::Device &device,
                         GraphicsSystem &sys ) {
  /*
  const MyGL::AbstractCamera & camera = scene.camera();

  MyGL::Render render( device,
                       sys.sceneCopy,
                       sys.mainDepth,
                       sys.glowData.vs,
                       sys.glowData.fs );
  render.clear( MyGL::Color(0,0,0,1) );

  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::AbstractGraphicObject& ptr = v[i].object();

    if( scene.viewTester().isVisible( ptr, camera ) ){
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }
*/
  }
