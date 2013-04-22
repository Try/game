#include "resourcebehavior.h"

#include "game/gameobject.h"
#include "game/world.h"

ResourceBehavior::ResourceBehavior( GameObject & o,
                                    Behavior::Closure &  ):obj(o) {
  isBusy = 0;
  obj.world().addResouce( &obj );
  }

ResourceBehavior::~ResourceBehavior() {
  obj.world().delResouce( &obj );
  }

void ResourceBehavior::tick(const Terrain &/*terrain*/) {
  /*
  static unsigned x = 0;
  if( isBusy )
    obj.rotate( x );

  x+=5;
  */
  }
