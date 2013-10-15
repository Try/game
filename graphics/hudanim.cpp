#include "hudanim.h"

#include "game/world.h"
#include "game.h"

#include "behavior/buildingbehavior.h"

HudAnim::HudAnim( Scene & s,
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


StormEfect::StormEfect( Scene & ,
                        World       & w,
                        const ProtoObject &,
                        const PrototypesLoader &,
                        Resource &,
                        Physics  & ):w(w) {
  t = 3*Game::ticksPerSecond;
  }

void StormEfect::setPosition(int ix, int iy, float /*z*/) {
  x = ix;
  y = iy;
  }

void StormEfect::tick() {
  t -= 1;

  int r = 3, rq = r*Terrain::quadSize;
  int team = 0;

  if( t%3==0 )
    w.spatial().visit( x, y,
                       r,
                       &findEnemy,
                       x,
                       y,
                       rq,
                       team );
  }

bool StormEfect::isEnd() const {
  return t<=0;
  }

void StormEfect::findEnemy( GameObject &tg,
                            int x, int y,
                            int r,
                            int team ) {
  int d  = tg.distanceSQ( x, y );

  if( tg.team() == team )
    return;

  if( d<r*r && !tg.behavior.find<BuildingBehavior>())
    tg.setHP( tg.hp()-1 );
  }
