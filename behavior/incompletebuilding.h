#ifndef INCOMPLETEBUILDING_H
#define INCOMPLETEBUILDING_H

#include "abstractbehavior.h"
#include "behavior.h"
#include "buildingbehavior.h"

class WorkerBehavior;

class IncompleteBuilding : public BuildingBehavior  {
  public:
    IncompleteBuilding( GameObject & obj,
                        Behavior::Closure & c );
    ~IncompleteBuilding();


    void tick( const Terrain & terrain );

  private:
    GameObject & obj;
    int time;
    float tmax;
  };

#endif // INCOMPLETEBUILDING_H
