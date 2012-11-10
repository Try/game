#include "mainpass.h"

#include <MyGL/Render>
#include <cmath>

#include "mainmaterial.h"

using namespace MyGL;

MainPass::MainPass( Buffer & b,
                    Texture2d &d,
                    const VertexShader &v,
                    const FragmentShader &f )
  :buffer( b ), depth(d), vs(v), fs(f) {
  }

void MainPass::exec( const Scene & scene,
                     const Scene::Objects &v, Device & device ){
  /*
  const AbstractCamera & camera = scene.camera();

  Render render( device,
                 buffer.data, 4,
                 depth,
                 vs, fs );

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

    float view[3] = { float(vm.at(2,0)),
                      float(vm.at(2,1)),
                      float(vm.at(2,2)) };
    float len = sqrt(view[0]*view[0] + view[1]*view[1] + view[2]*view[2]);

    for( int i=0; i<3; ++i )
      view[i] /= len;

    device.setUniform( fs, view, 3, "view" );
    }

  size_t count = 0;
  for( size_t i=0; i<v.size(); ++i ){
    const AbstractGraphicObject& ptr = v[i].object();

    if( scene.viewTester().isVisible( ptr, camera ) ){
      render.draw( v[i].material(), ptr,
                   ptr.transform(), camera );
      ++count;
      }
    }

  count = 0;
  */
  }

AbstractMaterial::MaterialID MainPass::materialId() const{
  return AbstractMaterial::materialid<MainMaterial>();
  }
