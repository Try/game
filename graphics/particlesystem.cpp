#include "particlesystem.h"

#include "particlesystemengine.h"

#include <cmath>

ParticleSystem::ParticleSystem() {
  actualOXYZ = false;
  setPosition(0,0,0);
  proto  = 0;
  engine = 0;
  dispathMode = false;  
  }

void ParticleSystem::evalute( ParticleSystem::Point3 &p ) {
  ParticleSystemDeclaration::D d = ParticleSystemDeclaration::mix(decl.dmin, decl.dmax);
  evalute(p, d);
  }

void ParticleSystem::evalute( ParticleSystem::Point3 &p,
                              ParticleSystemDeclaration::D& d ) {
  p.x += d.x;
  p.y += d.y;
  p.z += d.z;

  p.size  += d.size;
  p.angle += d.angle;

  p.color.set( p.color.r() + d.r,
               p.color.g() + d.g,
               p.color.b() + d.b,
               p.color.a() + d.a );
  }

ParticleSystem::ParticleSystem( ParticleSystemEngine & e,
                                const ProtoObject::View &p,
                                const ParticleSystemDeclaration &decl )
               :decl(decl), engine(&e){
  setPosition(0,0,0);
  par.reserve(128);
  proto = &p;
  dispathMode = false;
  engine->particles.push_back( this );
  }

ParticleSystem::ParticleSystem(const ParticleSystem &other) {
  *this = other;
  dispathMode = false;
  engine->particles.push_back( this );
  }

ParticleSystem &ParticleSystem::operator =  (const ParticleSystem &other) {
  engine = other.engine;
  proto  = other.proto;
  dispathMode = other.dispathMode;

  setPosition( other.x(), other.y(), other.z() );
  par  = other.par;
  decl = other.decl;

  return *this;
  }

ParticleSystem::~ParticleSystem() {
  for( size_t i=0; i<engine->particles.size(); ++i )
    if( engine->particles[i]==this ){
      engine->particles[i] = engine->particles.back();
      engine->particles.pop_back();

      ParticleSystem *p = new ParticleSystem();
      *p = *this;
      p->dispathMode = true;

      engine->dispath.push_back( std::shared_ptr<ParticleSystem>(p) );
      return;
      }
  }

void ParticleSystem::exec( int dt ) {
  for( size_t i=0; i<par.size(); ++i ){
    engine->emitParticle( par[i].x,
                          par[i].y,
                          par[i].z,
                          par[i].size,
                          par[i].angle,
                          par[i].color );
    }

  if( dt==0 )
    return;

  if( !dispathMode ){
    int c = decl.density;
    if( decl.density<0 && rand()%(-decl.density) )
      c = 1;

    float a  = 1.0/std::max(c-1,1),
          dx = mx-ox,
          dy = my-oy,
          dz = mz-oz;

    for( int i=0; i<c; ++i ){
      par.push_back( Point3( ox+dx*i*a, oy+dy*i*a, oz+dz*i*a ) );
      ParticleSystemDeclaration::D d = ParticleSystemDeclaration::mix( decl.initMin,
                                                                       decl.initMax );
      evalute(par.back(), d);
      par.back().color.set( d.r, d.g, d.b, d.a );
      }
    }

  for( size_t i=0; i<par.size(); ++i ){
    evalute( par[i] );
    }

  for( size_t i=0; i<par.size(); ){
    if( par[i].size<0.005 || par[i].color.a()<0.01 ){
      par[i] = par.back();
      par.pop_back();
      } else {
      ++i;
      }
    }

  ox = mx;
  oy = my;
  oz = mz;
  actualOXYZ = true;
  }

void ParticleSystem::setPosition(float ix, float iy, float iz) {
  mx = ix;
  my = iy;
  mz = iz;

  if( !actualOXYZ ){
    ox = mx;
    oy = my;
    oz = mz;
    }
  }

float ParticleSystem::x() const {
  return mx;
  }

float ParticleSystem::y() const {
  return my;
  }

float ParticleSystem::z() const {
  return mz;
  }

const ProtoObject::View &ParticleSystem::viewInfo() const {
  return *proto;
  }

ParticleSystem::Point3::Point3(float x, float y, float z):
  x(x), y(y), z(z), size(0), angle(0) {

  }
