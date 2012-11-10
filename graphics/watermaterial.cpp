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
  table.add( 0.1, "specularFactor", MyGL::UniformTable::Fragment );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }

  return true;
  }

void WaterMaterial::exec( const MyGL::Scene &scene,
                          const MyGL::Scene::Objects &v,
                          MyGL::Device &device,
                          GraphicsSystem &sys ) {
  /*
  const MyGL::AbstractCamera & camera = scene.camera();

  MyGL::FragmentShader & fs = sys.displaceData.fsWater;
  MyGL::VertexShader   & vs = sys.displaceData.vsWater;

  MyGL::Render render( device,
                       sys.gbuffer.buffer(0),
                       sys.mainDepth,
                       vs,
                       fs );

  if( scene.lights().direction().size()>0 ){
    MyGL::DirectionLight l = scene.lights().direction()[0];

    float dir[3] = { float(l.xDirection()),
                     float(l.yDirection()),
                     float(l.zDirection()) };
    device.setUniform( fs, dir, 3, "lightDirection" );

    float color[3] = { l.color().r(),
                       l.color().g(),
                       l.color().b() };
    device.setUniform( fs, color, 3, "lightColor" );

    float ablimient[3] = { l.ablimient().r(),
                           l.ablimient().g(),
                           l.ablimient().b() };
    device.setUniform( fs, ablimient, 3, "lightAblimient" );

    MyGL::Matrix4x4 vm = camera.view();

    float view[3] = { float(vm.at(2,0)), float(vm.at(2,1)), float(vm.at(2,2)) };
    float len = sqrt(view[0]*view[0] + view[1]*view[1] + view[2]*view[2]);

    for( int i=0; i<3; ++i )
      view[i] /= len;

    device.setUniform( fs, view, 3, "view" );
    }

  device.setUniform( sys.displaceData.fsWater,
                     sys.sceneCopy.surface(),
                     "scene" );
  device.setUniform( sys.displaceData.fsWater,
                     sys.waterNormals.surface(),
                     "normalMap" );*/
/*
  device.setUniform( sys.displaceData.fsWater,
                     sys.depthSmBuffer,
                     "sceneDepth" );
*/ /*
  float tc[] = { 1.0f/sys.sceneCopy.width(), 1.0f/sys.sceneCopy.height() };
  device.setUniform( sys.displaceData.fsWater, tc, 2, "dTexCoord");

  static unsigned dc = 0;
  float tw[] = { (dc)/10000.0f, (dc)/10000.0f };
  dc = (dc+1)%10000;
  device.setUniform( sys.displaceData.fsWater, tw, 2, "dWaterCoord");

  for( size_t i=0; i<v.size(); ++i ){
    const MyGL::AbstractGraphicObject& ptr = v[i].object();

    if( scene.viewTester().isVisible( ptr, camera ) ){
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      }
    }
*/
  }
