#include "factory.h"

#include "behavior/abstractbehavior.h"
#include "behavior/movebehavior.h"
#include "behavior/buildingbehavior.h"
#include "behavior/resourcebehavior.h"
#include "behavior/workerbehavior.h"
#include "behavior/warehousebehavior.h"
#include "behavior/recruterbehavior.h"
#include "behavior/builderbehavior.h"
#include "behavior/incompletebuilding.h"
#include "behavior/warriorbehavior.h"

void initFactorys(){
#define behavior(T,N) BehaviorsFactory::addProduct<T>( N )
  behavior(MoveBehavior,       "move"       );
  behavior(BuildingBehavior,   "building"   );
  behavior(ResourceBehavior,   "resource"   );
  behavior(WorkerBehavior,     "worker"     );
  behavior(WarehouseBehavior,  "warehouse"  );
  behavior(RecruterBehavior,   "recruter"   );
  behavior(BuilderBehavior,    "builder"    );
  behavior(IncompleteBuilding, "incomplete" );
  behavior(WarriorBehavior,    "warrior"    );
#undef behavior
  }
