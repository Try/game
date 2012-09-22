#include "warriorbehavior.h"
#include "game/gameobject.h"
#include "game.h"

WarriorBehavior::WarriorBehavior( GameObject &obj,
                                  Behavior::Closure &c ):obj(obj) {
  isAtk    = false;
  dAtkTime = 0;

  lastX = obj.x();
  lastY = obj.y();
  }

WarriorBehavior::~WarriorBehavior() {

  }

void WarriorBehavior::tick( const Terrain & ) {
  if( dAtkTime>0 )
    --dAtkTime;

  if( dAtkTime )
    return;

  size_t id = -1;
  int dist = -1;

  for( size_t i=0; i<obj.world().objectsCount(); ++i ){
    GameObject & tg = obj.world().object(i);
    int d = tg.distanceSQ( obj.x(), obj.y() );

    if( tg.team()!=obj.team() &&
        tg.team()!=0 &&
        (dist < 0 || d<dist) ){
      id   = i;
      dist = d;
      }
    }

  if( !obj.isOnMove() || isAtk || (obj.distanceQL(lastX, lastY)>1) ){
    if( id!=size_t(-1) )
      taget = obj.world().objectWPtr(id);

    if( taget ){
      int vrange = obj.getClass().data.visionRange*Terrain::quadSize;
      int arange = (0 + obj.getClass().data.size
                      + taget.value().getClass().data.size )*Terrain::quadSize;

      vrange = vrange*vrange;
      arange = arange*arange;
      int d = taget.value().distanceSQ(obj.x(), obj.y());
      if( d <= arange )
        damageTo( taget.value() ); else
      if( d <= vrange )
        move( taget.value().x(), taget.value().y() );
      }
    }
  }

bool WarriorBehavior::message( AbstractBehavior::Message msg,
                               int /*x*/, int /*y*/,
                               AbstractBehavior::Modifers /*md*/) {
  if( msg==Move ||
      msg==MoveGroup ||
      msg==MineralMove ||
      msg==MoveSingle ){
    isAtk = false;
    }

  return false;
  }

void WarriorBehavior::move(int x, int y) {
  int qs = Terrain::quadSize;

  x /= qs;
  y /= qs;
  obj.behavior.message( MoveSingle, x*qs + qs/2, y*qs + qs/2 );

  isAtk = true;

  dAtkTime = 100;
  lastX = obj.x();
  lastY = obj.y();
  }

void WarriorBehavior::damageTo(GameObject &dobj) {
  dobj.setHP( dobj.hp() - 1 );
  obj.setViewDirection( dobj.x(), dobj.y() );
  }
