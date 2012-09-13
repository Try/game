#include "warriorbehavior.h"
#include "game/gameobject.h"
#include "game.h"

WarriorBehavior::WarriorBehavior( GameObject &obj,
                                  Behavior::Closure &c ):obj(obj) {

  }

WarriorBehavior::~WarriorBehavior() {

  }

void WarriorBehavior::tick( const Terrain & ) {
  size_t id = -1;
  int dist = -1;

  for( size_t i=0; i<obj.world().objectsCount(); ++i ){
    GameObject & tg = obj.world().object(i);
    int d = tg.distanceQ( obj.x(), obj.y() );

    if( tg.team()!=obj.team() &&
        tg.team()!=0 &&
        (dist < 0 || d<dist) ){
      id   = i;
      dist = d;
      }
    }

  if( !obj.isOnMove() ){
    if( id!=size_t(-1) && taget!=obj.world().objectWPtr(id) )
      taget = obj.world().objectWPtr(id);

    if( taget )
      move( taget.value().x(), taget.value().y() );
    }
  }

void WarriorBehavior::move(int x, int y) {
  int qs = Terrain::quadSize;

  x /= qs;
  y /= qs;
  obj.behavior.message( MoveSingle, x*qs + qs/2, y*qs + qs/2 );
  }
