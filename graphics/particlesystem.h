#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include "game/protoobject.h"

class ParticleSystemEngine;

class ParticleSystem {
  public:
    ParticleSystem( ParticleSystemEngine & engine,
                    const ProtoObject::View& proto );
    ParticleSystem( const ParticleSystem& other );
    virtual ~ParticleSystem();

    ParticleSystem& operator = ( const ParticleSystem& );

    void exec();

    void setPosition( float x, float y, float z );
    float x() const;
    float y() const;
    float z() const;

    const ProtoObject::View& viewInfo() const;
  private:
    float mx, my, mz;
    ParticleSystemEngine * engine;
    const ProtoObject::View* proto;

    struct Point3{
      Point3( float x, float y, float z );
      float x,y,z;
      float size;
      };

    std::vector<Point3> par;
  };

#endif // PARTICLESYSTEM_H
