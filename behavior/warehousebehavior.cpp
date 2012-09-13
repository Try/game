#include "warehousebehavior.h"

#include "game/gameobject.h"
#include "game/world.h"

WarehouseBehavior::WarehouseBehavior( GameObject & o,
                                      Behavior::Closure & c ) : obj(o){
  obj.world().addWarehouse( &obj );
  }

WarehouseBehavior::~WarehouseBehavior() {
  obj.world().delWarehouse( &obj );
  }

void WarehouseBehavior::tick(const Terrain &terrain) {
  }
