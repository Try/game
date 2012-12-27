#ifndef PARTICLESYSTEMENGINE_H
#define PARTICLESYSTEMENGINE_H

#include <MyGL/GraphicObject>
#include <vector>

#include <MyWidget/signal>
#include "game/protoobject.h"
#include "prototypesloader.h"
#include "resource.h"

namespace MyGL{
  class Scene;
  class AbstractGraphicObject;
  }

class Resource;
class ParticleSystem;

class ParticleSystemEngine {
  public:
    ParticleSystemEngine( MyGL::Scene & s,
                          const PrototypesLoader & p,
                          Resource & r );

    void exec();
    MyWidget::signal< MyGL::AbstractGraphicObject &,
                      const ProtoObject::View &,
                      const MyGL::Color & > setupMaterial;
  private:
    MyGL::Scene & scene;
    const PrototypesLoader & proto;
    Resource & res;

    std::vector<MyGL::GraphicObject> view;
    std::vector<ParticleSystem*> particles, visible;

    void emitParticle( Model::Raw &v,
                       float x, float y, float z , float sz);

    void emitParticle(float x, float y, float z , float sz);

    double left[3], top[3], norm[3];
    Model::Raw raw;

    friend class ParticleSystem;

    static bool cmpMat( const ParticleSystem* a,
                        const ParticleSystem* b );
  };

#endif // PARTICLESYSTEMENGINE_H
