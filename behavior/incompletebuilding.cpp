#include "incompletebuilding.h"

#include "game/gameobject.h"
#include "game/world.h"

#include "game/gameobject.h"
#include "game.h"

IncompleteBuilding::IncompleteBuilding( GameObject & o, Behavior::Closure & c )
                   :BuildingBehavior(o,c), obj(o) {
  time = obj.getClass().data.buildTime;
  tmax = time;

  obj.player().incLim( -obj.getClass().data.limInc );

  obj.behavior.clear();
  }

IncompleteBuilding::~IncompleteBuilding() {
  obj.player().incLim( obj.getClass().data.limInc );
  }

void IncompleteBuilding::tick(const Terrain &t) {
  BuildingBehavior::tick(t);
  if( time>0 )
    --time;

  float wx = obj.x()/Terrain::quadSizef,
        wy = obj.y()/Terrain::quadSizef;

  obj.setPosition( obj.x(), obj.y(),
                   obj.world().terrain().heightAt(wx,wy) +
                   World::coordCastD(-time * obj.viewHeight()/tmax) );
  if( time<=0 ){
    GameObject & ptr = obj;

    ptr.behavior.del(this);
    //ptr.player().incLim( ptr.getClass().data.limInc );

    const ProtoObject & p = ptr.getClass();
    for( auto i=p.behaviors.begin(); i!=p.behaviors.end(); ++i ){
      ptr.behavior.add( *i );
      }
    }
  }
