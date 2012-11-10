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

  MyGL::Matrix4x4 sh = *shadowMatrix;
  sh.mul( object );

  table.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex );
  table.add( object,  "objectMatrix", MyGL::UniformTable::Vertex );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }

  return true;
  }

void DisplaceMaterial::exec( const MyGL::Scene &scene,
                             const MyGL::Scene::Objects &v,
                             MyGL::Device &device,
                             GraphicsSystem &sys ) {
  /*
  const MyGL::AbstractCamera & camera = scene.camera();

  MyGL::Render render( device,
                       sys.gbuffer.buffer(0),
                       sys.sceneCopyDepth,
                       sys.displaceData.vs,
                       sys.displaceData.fs );
  device.clearZ(1);

  device.setUniform( sys.displaceData.fs,
                     sys.sceneCopy.surface(),
                     "scene" );*/
/*
  device.setUniform( sys.displaceData.fs,
                     sys.mainDepth,
                     "sceneDepth" );*/
/*
  float tc[] = { 1.0f/sys.sceneCopy.width(), 1.0f/sys.sceneCopy.height() };
  device.setUniform( sys.displaceData.fs, tc, 2, "dTexCoord");


  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::AbstractGraphicObject& ptr = v[i].object();

    if( scene.viewTester().isVisible( ptr, camera ) ){
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }
*/
  }
