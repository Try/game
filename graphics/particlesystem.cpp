#include "particlesystem.h"

#include "particlesystemengine.h"

ParticleSystem::ParticleSystem(ParticleSystemEngine & e,
                                const ProtoObject::View &p ):engine(&e){
  setPosition(0,0,0);
  proto = &p;
  engine->particles.push_back( this );
  }

ParticleSystem::ParticleSystem(const ParticleSystem &other) {
  *this = other;
  engine->particles.push_back( this );
  }

ParticleSystem &ParticleSystem::operator =  (const ParticleSystem &other) {
  engine = other.engine;
  proto  = other.proto;

  setPosition( other.x(), other.y(), other.z() );

  return *this;
  }

ParticleSystem::~ParticleSystem() {
  for( size_t i=0; i<engine->particles.size(); ++i )
    if( engine->particles[i]==this ){
      engine->particles[i] = engine->particles.back();
      engine->particles.pop_back();
      }
  }

void ParticleSystem::exec() {
  for( size_t i=0; i<par.size(); ++i ){
    engine->emitParticle( par[i].x,
                          par[i].y,
                          par[i].z,
                          par[i].size );
    }

  if( viewInfo().name=="fire" ){
    par.push_back( Point3( x(), y(), z() ) );
    par.back().size = 0.03+0.03*( rand()/float(RAND_MAX) );

    par.back().x += 0.1*( rand()/float(RAND_MAX) - 0.5);
    par.back().y += 0.1*( rand()/float(RAND_MAX) - 0.5);

    for( size_t i=0; i<par.size(); ++i ){
      par[i].z    += 0.015;
      par[i].size -= 0.001;
      }

    for( size_t i=0; i<par.size(); ){
      if( par[i].size<0.005 ){
        par[i] = par.back();
        par.pop_back();
        } else {
        ++i;
        }
      }
    }

  if( viewInfo().name=="smoke" ){
    if( rand()%3==0 ){
      par.push_back( Point3( x(), y(), z() ) );
      par.back().size = 0.05;
      }

    for( size_t i=0; i<par.size(); ++i ){
      par[i].z    += 0.015;
      par[i].size += 0.001;

      par[i].x += 0.01*( rand()/float(RAND_MAX) - 0.5);
      par[i].y += 0.01*( rand()/float(RAND_MAX) - 0.5);
      }

    for( size_t i=0; i<par.size(); ){
      if( par[i].size>0.1 ){
        par[i] = par.back();
        par.pop_back();
        } else {
        ++i;
        }
      }
    }
  }

void ParticleSystem::setPosition(float ix, float iy, float iz) {
  mx = ix;
  my = iy;
  mz = iz;
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
  x(x), y(y), z(z), size(0) {

  }
