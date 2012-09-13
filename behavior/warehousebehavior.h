#ifndef WAREHOUSEBEHAVIOR_H
#define WAREHOUSEBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"

class WarehouseBehavior : public AbstractBehavior  {
  public:
    WarehouseBehavior( GameObject & o,
                       Behavior::Closure & c );
    ~WarehouseBehavior();


    void tick( const Terrain & terrain );

  private:
    GameObject & obj;
  };

#endif // WAREHOUSEBEHAVIOR_H
