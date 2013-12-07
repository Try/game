#include "particlesystemengine.h"

#include "resource.h"
#include <cmath>
#include "particlesystem.h"

#include <algorithm>

#include "scene.h"

ParticleSystemEngine::ParticleSystemEngine( Scene &s,
                                            const PrototypesLoader &p,
                                            Resource & r )
  :scene(s), proto(p), res(r) {
  raw.vertex.reserve(1024*32);
  visible.reserve( 128 );
  view.reserve(128);
  particles.reserve(128);
  }

ParticleSystemEngine::~ParticleSystemEngine() {
  }

void ParticleSystemEngine::exec( const Tempest::Matrix4x4 &mview,
                                 const Tempest::Matrix4x4 &mproj,
                                 int dt,
                                 bool invCullMode ) {
  view.clear();

  Tempest::Matrix4x4 mvp  = mproj;
  Tempest::Matrix4x4 vmat = mview;
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

  if( invCullMode ){
    ll = -ll;
    ln *= -0.5;
    }

  for( int i=0; i<3; ++i ){
    left[i] /= ll;
    top [i] /= lt;
    norm[i] /= ln;
    }

  Tempest::ModelBounds bds;
  std::fill( bds.min, bds.min+3, -0.5 );
  std::fill( bds.mid, bds.mid+3,  0   );
  std::fill( bds.max, bds.max+3,  0.5 );

  visible.clear();
  for( size_t i=0; i<particles.size(); ++i ){
    ParticleSystem &p = *particles[i];
    Tempest::ModelBounds bds1 = bds;

    float dpos[3] = {p.x(), p.y(), p.z()};
    for( int r=0; r<3; ++r ){
      bds1.min[r] += dpos[r];
      bds1.mid[r] += dpos[r];
      bds1.max[r] += dpos[r];
      }

    if( scene.viewTester().isVisible( bds1, mvp ) )
      visible.push_back( &p );
    }

  for( size_t i=0; i<dispath.size(); ++i ){
    ParticleSystem &p = *dispath[i];
    Tempest::ModelBounds bds1 = bds;

    float dpos[3] = {p.x(), p.y(), p.z()};
    for( int r=0; r<3; ++r ){
      bds1.min[r] += dpos[r];
      bds1.mid[r] += dpos[r];
      bds1.max[r] += dpos[r];
      }

    if( scene.viewTester().isVisible( bds1, mvp ) )
      visible.push_back( &p ); else
      dispath[i].reset();
    }

  {
    size_t dc = 0;
    for( size_t i=0; i<dispath.size(); ++i ){
      if( dispath[i] ){
        dispath[dc] = dispath[i];
        ++dc;
        }
      }

    dispath.resize(dc);
  }

  std::sort( visible.begin(), visible.end(), cmpMat );

  raw.vertex.clear();
  const ProtoObject::View *currView = 0;
  for( size_t i=0; i<visible.size(); ++i ){
    ParticleSystem &p = *visible[i];

    if( currView==0 ||
        currView->name != p.viewInfo().name ){
      if( currView && raw.vertex.size() ){
        GraphicObject obj(scene);
        res.model( raw ).setTo( obj );

        setupMaterial( obj, *currView, Tempest::Color() );
        view.push_back( obj );

        raw.vertex.clear();
        }

      currView = &p.viewInfo();
      }

    p.exec( dt );
    }

  if( currView && raw.vertex.size() ){
    GraphicObject obj(scene);
    res.model( raw ).setTo(obj);

    setupMaterial( obj, *currView, Tempest::Color() );
    view.push_back( obj );
    }

  for( size_t i=0; i<dispath.size(); ){
    if( dispath[i]->par.size()==0 ){
      std::swap( dispath[i],dispath.back() );
      dispath.pop_back();
      } else {
      ++i;
      }
    }
  }

void ParticleSystemEngine::update() {

  }

void ParticleSystemEngine::emitParticle( Model::Raw &raw,
                                         float x, float y, float z,
                                         float sz,
                                         float angle,
                                         ParticleSystemDeclaration::Orign orign,
                                         Tempest::Color & color ) {
  MVertex v;
  v.u = 0.5;
  v.v = 0.5;
  v.nx = -norm[0];
  v.ny = -norm[1];
  v.nz = -norm[2];

  v.color[0] = 255*color.r();
  v.color[1] = 255*color.g();
  v.color[2] = 255*color.b();
  v.color[3] = 255*color.a();

  v.bx = -left[0];
  v.by = -left[1];
  v.bz = -left[2];

  double mul[2] = {-1, 1};

  size_t iSz = raw.vertex.size();

  float sa = sin(angle), ca = cos(angle);

  float tn[3], ln[3];

  if( orign == ParticleSystemDeclaration::Screen )
    for( int i=0; i<3; ++i ){
      ln[i] = left[i]*ca - top[i]*sa;
      tn[i] = left[i]*sa + top[i]*ca;
      }

  for( int i=0; i<2; ++i )
    for( int r=0; r<2; ++r ){
      double dx = 0, dy = 0, dz = 0;
      if( orign == ParticleSystemDeclaration::Screen ){
        dx = mul[i]*ln[0] + mul[r]*tn[0];
        dy = mul[i]*ln[1] + mul[r]*tn[1];
        dz = mul[i]*ln[2] + mul[r]*tn[2];
        } else
      if( orign == ParticleSystemDeclaration::XYOrign ){
        dx = -mul[i]*1;
        dy =  mul[r]*1;
        dz = 0;
        } else
      if( orign == ParticleSystemDeclaration::Planar ){
        double t[3] = {0,0,1};
        dx = mul[i]*left[0] + mul[r]*t[0];
        dy = mul[i]*left[1] + mul[r]*t[1];
        dz = mul[i]*left[2] + mul[r]*t[2];
        }

      double sdx = sz*dx, sdy = sz*dy;

      v.x = x + sdx;//sdx*ca - sdy*sa;
      v.y = y + sdy;//sdx*sa + sdy*ca;
      v.z = z + sz*dz;

      v.nx = v.nx+0.25*dx;
      v.ny = v.ny+0.25*dy;
      v.nz = v.nz+0.25*dz;

      if( orign == ParticleSystemDeclaration::Screen ){
        v.x = v.x+0.5*sz*v.nx;
        v.y = v.y+0.5*sz*v.ny;
        v.z = v.z+0.5*sz*v.nz;
        }

      v.u = std::max(0.0, mul[i]);
      v.v = 1.0 - std::max(0.0, mul[r]);

      raw.vertex.push_back( v );
      }

  raw.vertex.push_back( raw.vertex.back() );
  raw.vertex.push_back( raw.vertex.back() );
  raw.vertex[iSz+3] = raw.vertex[iSz+1];
  raw.vertex[iSz+5] = raw.vertex[iSz+2];
  }

void ParticleSystemEngine::emitParticle( float x,
                                         float y,
                                         float z,
                                         float sz,
                                         float angle,
                                         ParticleSystemDeclaration::Orign orign,
                                         Tempest::Color & color ) {
  emitParticle(raw, x, y, z, sz, angle, orign, color );
  }

bool ParticleSystemEngine::cmpMat( const ParticleSystem *a,
                                   const ParticleSystem *b ) {
  return a->viewInfo().name < b->viewInfo().name;
  }
