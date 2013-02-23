#ifndef gSCENE_H
#define gSCENE_H

#include "material.h"

class Scene : public MyGL::AbstractScene<AbstractGraphicObject> {
  public:
    Scene();
    using MyGL::AbstractScene< AbstractGraphicObject >::Objects;

    const Objects& shadowCasters() const;
    const Objects& mainObjects() const;
    const Objects& glowObjects() const;
    const Objects& additiveObjects() const;

    const Objects& terrainMinorObjects() const;
    const Objects& displaceObjects() const;
    const Objects& waterObjects() const;
    const Objects& transparentObjects() const;
    const Objects& transparentZWObjects() const;

    const Objects& grassObjects() const;

    const Objects& fogOfWar() const;
    const Objects& omni() const;
  private:
    Objects shCast, mainObj, glowObj, terrainMinor,
            displaceObj, waterObj, addObj, transpObj,
            transpObjZW, fogOfWarObj,
            omniObj, grassObj;

    void onObjectAdded(AbstractGraphicObject *);
    void onObjectRemoved(AbstractGraphicObject *);
  };

#endif // SCENE_H
