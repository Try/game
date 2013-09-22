#include "particlesystemdeclaration.h"

#include <cstring>
#include <cstdlib>

ParticleSystemDeclaration::ParticleSystemDeclaration() {
  init( initMax, 1 );
  init( initMin, 1 );

  init( dmax );
  init( dmin );

  init( ddmax );
  init( ddmin );

  density = 1;
  hasDD   = false;
  }

void ParticleSystemDeclaration::init(ParticleSystemDeclaration::D &d, float c ) {
  memset( &d, 0, sizeof(d) );

  d.r = c;
  d.g = c;
  d.b = c;
  d.a = c;
  }

ParticleSystemDeclaration::D
  ParticleSystemDeclaration::mix( const ParticleSystemDeclaration::D &min,
                                  const ParticleSystemDeclaration::D &max ) {
  D ret;
  ret.x = mix( min.x, max.x, rand()/float(RAND_MAX) );
  ret.y = mix( min.y, max.y, rand()/float(RAND_MAX) );
  ret.z = mix( min.z, max.z, rand()/float(RAND_MAX) );

  ret.size = mix( min.size, max.size, rand()/float(RAND_MAX) );

  ret.r = mix( min.r, max.r, rand()/float(RAND_MAX) );
  ret.g = mix( min.g, max.g, rand()/float(RAND_MAX) );
  ret.b = mix( min.b, max.b, rand()/float(RAND_MAX) );
  ret.a = mix( min.a, max.a, rand()/float(RAND_MAX) );

  ret.angle = mix( min.angle, max.angle, rand()/float(RAND_MAX) );

  return ret;
  }

float ParticleSystemDeclaration::mix(const float min, const float max, float a) {
  return min*(1-a) + a*max;
  }
