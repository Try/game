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

HudAnim &HudAnim::setPosition(float x, float y, float z) {
  view.setPosition( World::coordCastD(x),
                    World::coordCastD(y) );
  view.setViewPosition( x,
                        y,
                        z );
  return *this;
  }

void HudAnim::tick() {
  t -= 5;

  float s = t/100.0;
  view.setViewSize(s,s,s);
  }

void HudAnim::setTimeout( int tin ) {
  t = tin;
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

  if( d<r*r && !tg.behavior.find<BuildingBehavior>() ){
    if( !tg.hasEfect(EfectBase::Strorm) ){
      tg.setHP( tg.hp()-1 );
      tg.addEfectFlg( EfectBase::Strorm );
      }
    }
  }

ForceFieldEfect::ForceFieldEfect( Scene &s,
                                  World &w,
                                  const ProtoObject &proto,
                                  const PrototypesLoader &pl,
                                  Resource &res,
                                  Physics &p )
  : w(w),
    view( s, w, proto, pl ) {
  t = 4*Game::ticksPerSecond;
  view.loadView( res, p, 0 );
  }

ForceFieldEfect::~ForceFieldEfect() {
  int size = 3;
  Terrain &terrain = w.terrain();
  terrain.editBuildingsMap( x - size/2+size%2,
                            y - size/2+size%2,
                            size, size, -1);
  }

void ForceFieldEfect::setPosition(int ix, int iy, float /*z*/) {
  x = ix/Terrain::quadSize;
  y = iy/Terrain::quadSize;

  int vx = x*Terrain::quadSize, vy = y*Terrain::quadSize;
  view.setPosition( vx, vy );
  view.setViewPosition( World::coordCast(vx),
                        World::coordCast(vy)  );
  view.tick();

  Terrain &terrain = w.terrain();

  int size = 3;
  terrain.editBuildingsMap( x - size/2+size%2,
                            y - size/2+size%2,
                            size, size, 1);
  }

void ForceFieldEfect::tick() {
  t -= 1;
  }

bool ForceFieldEfect::isEnd() const {
  return t<=0;
  }
