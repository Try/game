#ifndef RESOURCEBEHAVIOR_H
#define RESOURCEBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"

class WorkerBehavior;

class ResourceBehavior : public AbstractBehavior  {
  public:
    ResourceBehavior( GameObject & obj,
                      Behavior::Closure & c );
    ~ResourceBehavior();


    void tick( const Terrain & terrain );

    WorkerBehavior* isBusy;

  private:
    GameObject & obj;
  };

#endif // RESOURCEBEHAVIOR_H
