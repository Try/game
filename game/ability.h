#ifndef ABILITY_H
#define ABILITY_H

#include "behavior/behaviormsgqueue.h"

class GameObject;
class World;

class Ability {
    Ability();

  public:
    static bool spell( Game &game, World &w,
                       GameObject &obj,
                       const BehaviorMSGQueue::MSG &m );

  private:
    static bool blink( Game &game, World &w,
                       GameObject &obj,
                       const BehaviorMSGQueue::MSG &m );
    static bool fog  ( Game &game, World &w,
                       GameObject &obj,
                       const BehaviorMSGQueue::MSG &m );
  };

#endif // ABILITY_H
