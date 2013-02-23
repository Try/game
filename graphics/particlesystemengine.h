#ifndef PARTICLESYSTEMENGINE_H
#define PARTICLESYSTEMENGINE_H

#include <MyGL/GraphicObject>
#include <vector>

#include <MyWidget/signal>
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

    void exec( const MyGL::Matrix4x4 &view,
               const MyGL::Matrix4x4 &proj,
               int dt,
               bool invCullMode = false );
    void update();

    MyWidget::signal< AbstractGraphicObject &,
                      const ProtoObject::View &,
                      const MyGL::Color & > setupMaterial;
  private:
    Scene & scene;
    const PrototypesLoader & proto;
    Resource & res;

    std::vector<GraphicObject> view;
    std::vector<ParticleSystem*> particles, visible;
    std::vector< std::unique_ptr<ParticleSystem> > dispath;

    void emitParticle( Model::Raw &v,
                       float x, float y, float z , float sz,
                       MyGL::Color &color );

    void emitParticle( float x, float y, float z, float sz,
                       MyGL::Color &color );

    double left[3], top[3], norm[3];
    Model::Raw raw;

    friend class ParticleSystem;

    static bool cmpMat( const ParticleSystem* a,
                        const ParticleSystem* b );
  };

#endif // PARTICLESYSTEMENGINE_H
