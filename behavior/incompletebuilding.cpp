#include "incompletebuilding.h"

#include "game/gameobject.h"
#include "game/world.h"

#include "game/gameobject.h"
#include "game.h"

IncompleteBuilding::IncompleteBuilding( GameObject & o, Behavior::Closure & c )
                   :BuildingBehavior(o,c), obj(o) {
  time = 100;
  obj.behavior.clear();
  }

IncompleteBuilding::~IncompleteBuilding() {
  }

void IncompleteBuilding::tick(const Terrain &t) {
  BuildingBehavior::tick(t);
  if( time>0 )
    --time;

  obj.setPosition( obj.x(), obj.y(),
                   World::coordCastD(-time * obj.viewHeight() /100.0) );
  if( time<=0 ){
    GameObject & ptr = obj;

    ptr.behavior.del(this);

    const ProtoObject & p = ptr.getClass();
    for( auto i=p.behaviors.begin(); i!=p.behaviors.end(); ++i ){
      ptr.behavior.add( *i );
      }
    }
  }
