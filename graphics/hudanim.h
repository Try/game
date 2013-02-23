#ifndef HUDANIM_H
#define HUDANIM_H

#include "game/gameobjectview.h"

class HudAnim {
  public:
    HudAnim( Scene & s,
             World       & wrld,
             const ProtoObject &p,
             const PrototypesLoader & pl,

             Resource & res,
             Physics  & phys );

    void setPosition( float x, float y, float z );

    void tick();
    bool isEnd() const;
  private:
    GameObjectView view;

    int t;
  };

#endif // HUDANIM_H
