#include "hudanim.h"

#include "game/world.h"

HudAnim::HudAnim( MyGL::Scene & s,
                  World       & wrld,
                  const ProtoObject &p,
                  const PrototypesLoader & pl,
                  Resource &res, Physics &phys )
  :view(s, wrld, p, pl ) {
  view.loadView(res, phys, 0);
  //view.setViewSize(1,1,1);

  t = 100;
  }

void HudAnim::setPosition(float x, float y, float z) {
  view.setPosition( World::coordCastD(x),
                    World::coordCastD(y) );
  view.setViewPosition( x,
                        y,
                        z );
  }

void HudAnim::tick() {
  t -= 5;

  float s = t/100.0;
  view.setViewSize(s,s,s);
  }

bool HudAnim::isEnd() const {
  return t<=0;
  }
