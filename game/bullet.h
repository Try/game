#ifndef BULLET_H
#define BULLET_H

#include "gameobject.h"

class Bullet {
  public:
    Bullet( Scene &s,
            World &wrld,
            const ProtoObject &p,
            const PrototypesLoader &pl);

    GameObjectView view;

    int x,y, tgX, tgY;
    double z, tgZ;
    int speed;

    bool isFinished;
    int  absDmg;

    void tick();

    float mvec[3];
  private:
    double l0;
  };

#endif // BULLET_H
