#ifndef PARTICLESYSTEMENGINE_H
#define PARTICLESYSTEMENGINE_H

#include <Tempest/GraphicObject>
#include <vector>

#include <Tempest/signal>
#include "game/protoobject.h"
#include "prototypesloader.h"
#include "resource.h"
#include "graphics/material.h"

class Scene;
class Resource;
class ParticleSystem;

class ParticleSystemEngine {
  public:
    ParticleSystemEngine( Scene & s,
                          const PrototypesLoader & p,
                          Resource & r );

    void exec( const Tempest::Matrix4x4 &view,
               const Tempest::Matrix4x4 &proj,
               int dt,
               bool invCullMode = false );
    void update();

    Tempest::signal< AbstractGraphicObject &,
                      const ProtoObject::View &,
                      const Tempest::Color & > setupMaterial;
  private:
    Scene & scene;
    const PrototypesLoader & proto;
    Resource & res;

    std::vector<GraphicObject> view;
    std::vector<ParticleSystem*> particles, visible;
    std::vector< std::shared_ptr<ParticleSystem> > dispath;

    void emitParticle(Model::Raw &v,
                       float x, float y, float z , float sz, float angle,
                       Tempest::Color &color );

    void emitParticle(float x, float y, float z, float sz, float angle,
                       Tempest::Color &color );

    double left[3], top[3], norm[3];
    Model::Raw raw;

    friend class ParticleSystem;

    static bool cmpMat( const ParticleSystem* a,
                        const ParticleSystem* b );
  };

#endif // PARTICLESYSTEMENGINE_H
