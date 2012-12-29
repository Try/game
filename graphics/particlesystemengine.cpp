#include "particlesystemengine.h"

#include "resource.h"
#include <cmath>
#include <MyGL/Scene>
#include "particlesystem.h"

#include <algorithm>

ParticleSystemEngine::ParticleSystemEngine(MyGL::Scene &s,
                                            const PrototypesLoader &p,
                                            Resource & r )
  :scene(s), proto(p), res(r) {
  raw.vertex.reserve(2048);
  visible.reserve( 128 );
  }

void ParticleSystemEngine::exec() {
  view.clear();

  MyGL::Matrix4x4 mvp = scene.camera().projective();
  MyGL::Matrix4x4 vmat = scene.camera().view();
  mvp.mul( vmat );

  double ileft[3] = { vmat.data()[0], vmat.data()[4], vmat.data()[8] };
  double  itop[3] = { vmat.data()[1], vmat.data()[5], vmat.data()[9] };
  double inorm[3] = { vmat.data()[2], vmat.data()[6], vmat.data()[10] };

  std::copy( ileft, ileft+3, left );
  std::copy( itop,  itop +3, top  );
  std::copy( inorm, inorm+3, norm );

  float ll = left[0]*left[0]+left[1]*left[1]+left[2]*left[2];
  float lt =  top[0]* top[0]+ top[1]* top[1]+ top[2]* top[2];
  float ln = norm[0]*norm[0]+norm[1]*norm[1]+norm[2]*norm[2];

  ll = 0.5*sqrt(ll);
  lt = 0.5*sqrt(lt);
  ln = sqrt(ln);

  for( int i=0; i<3; ++i ){
    left[i] /= ll;
    top [i] /= lt;
    norm[i] /= ln;
    }

  MyGL::ModelBounds bds;
  std::fill( bds.min, bds.min+3, -0.5 );
  std::fill( bds.mid, bds.mid+3,  0   );
  std::fill( bds.max, bds.max+3,  0.5 );

  visible.clear();
  for( size_t i=0; i<particles.size(); ++i ){
    ParticleSystem &p = *particles[i];
    MyGL::ModelBounds bds1 = bds;

    float dpos[3] = {p.x(), p.y(), p.z()};
    for( int r=0; r<3; ++r ){
      bds1.min[r] += dpos[r];
      bds1.mid[r] += dpos[r];
      bds1.max[r] += dpos[r];
      }

    if( scene.viewTester().isVisible( bds1, mvp ) )
      visible.push_back( &p );
      //p.exec();
    }

  std::sort( visible.begin(), visible.end(), cmpMat );

  raw.vertex.clear();
  const ProtoObject::View *currView = 0;
  for( size_t i=0; i<visible.size(); ++i ){
    ParticleSystem &p = *visible[i];

    if( currView==0 ||
        currView->name != p.viewInfo().name ){
      if( currView && raw.vertex.size() ){
        MyGL::GraphicObject obj(scene);
        obj.setModel( res.model( raw ) );
        setupMaterial( obj, *currView, MyGL::Color() );
        view.push_back( obj );

        raw.vertex.clear();
        }

      currView = &p.viewInfo();
      }

    p.exec();
    }

  if( currView && raw.vertex.size() ){
    MyGL::GraphicObject obj(scene);
    obj.setModel( res.model( raw ) );
    setupMaterial( obj, *currView, MyGL::Color() );
    view.push_back( obj );
    }
  }

void ParticleSystemEngine::emitParticle( Model::Raw &raw,
                                         float x, float y, float z,
                                         float sz,
                                         MyGL::Color & color ) {
  MVertex v;
  v.u = 0.5;
  v.v = 0.5;
  v.normal[0] = -norm[0];
  v.normal[1] = -norm[1];
  v.normal[2] = -norm[2];

  v.color[0] = color.r();
  v.color[1] = color.g();
  v.color[2] = color.b();
  v.color[3] = color.a();

  double mul[2] = {-1, 1};

  size_t iSz = raw.vertex.size();

  for( int i=0; i<2; ++i )
    for( int r=0; r<2; ++r ){
      double dx = mul[i]*left[0] + mul[r]*top[0];
      double dy = mul[i]*left[1] + mul[r]*top[1];
      double dz = mul[i]*left[2] + mul[r]*top[2];

      v.x = x + sz*dx;
      v.y = y + sz*dy;
      v.z = z + sz*dz;

      v.normal[0] += 0.25*dx;
      v.normal[1] += 0.25*dy;
      v.normal[2] += 0.25*dz;

      v.x += 0.5*sz*v.normal[0];
      v.y += 0.5*sz*v.normal[1];
      v.z += 0.5*sz*v.normal[2];

      v.u = std::max(0.0, mul[i]);
      v.v = 1.0 - std::max(0.0, mul[r]);

      raw.vertex.push_back( v );
      }

  raw.vertex.push_back( raw.vertex.back() );
  raw.vertex.push_back( raw.vertex.back() );
  raw.vertex[iSz+3] = raw.vertex[iSz+1];
  raw.vertex[iSz+5] = raw.vertex[iSz+2];
  }

void ParticleSystemEngine::emitParticle( float x, float y, float z,
                                         float sz,
                                         MyGL::Color & color ) {
  emitParticle(raw, x, y, z, sz, color );
  }

bool ParticleSystemEngine::cmpMat( const ParticleSystem *a,
                                   const ParticleSystem *b ) {
  return a->viewInfo().name < b->viewInfo().name;
  }
