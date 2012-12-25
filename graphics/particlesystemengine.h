#ifndef PARTICLESYSTEMENGINE_H
#define PARTICLESYSTEMENGINE_H

#include <MyGL/GraphicObject>
#include <vector>

namespace MyGL{
  class Scene;
  }

class ParticleSystemEngine {
  public:
    ParticleSystemEngine(MyGL::Scene & s );

  private:
    MyGL::Scene & scene;
    std::vector<MyGL::GraphicObject> view;
  };

#endif // PARTICLESYSTEMENGINE_H
