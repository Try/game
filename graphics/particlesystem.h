#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include "game/protoobject.h"
#include "game/particlesystemdeclaration.h"

#include <cstdint>
#include <Tempest/Color>

class ParticleSystemEngine;

class ParticleSystem {
  public:
    ParticleSystem( ParticleSystemEngine & engine,
                    const ProtoObject::View& proto,
                    const ParticleSystemDeclaration& decl );
    ParticleSystem( const ParticleSystem& other );
    virtual ~ParticleSystem();

    ParticleSystem& operator = ( const ParticleSystem& );

    void exec(int dt);

    void setPosition( float x, float y, float z );
    float x() const;
    float y() const;
    float z() const;

    const ProtoObject::View& viewInfo() const;

  private:
    ParticleSystem();
    ParticleSystemDeclaration decl;

    float mx, my, mz;
    float ox, oy, oz;
    bool  actualOXYZ;

    ParticleSystemEngine * engine;
    const ProtoObject::View* proto;

    struct Point3{
      Point3( float x, float y, float z );
      float x,y,z;
      float size;
      float angle;

      Tempest::Color color;
      };

    std::vector<Point3> par;
    bool dispathMode;

    uint64_t time;

    void evalute( Point3& p );
    void evalute( Point3& p, ParticleSystemDeclaration::D& d );

  friend class ParticleSystemEngine;
  };

#endif // PARTICLESYSTEM_H
