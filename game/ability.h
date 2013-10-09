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

    static void autoCast(Game &game, World &w,
                          const std::string & spell , GameObject &obj);
  private:
    static bool blink( Game &game, World &w,
                       GameObject &obj,
                       const BehaviorMSGQueue::MSG &m );
    static bool fireStrike( Game &game, World &w,
                            GameObject &obj,
                            const BehaviorMSGQueue::MSG &m );
    static bool fireStrike( Game &game, World &w,
                            GameObject &obj,
                            GameObject &tg );
    static bool heal( Game &game, World &w,
                      GameObject &obj,
                      const BehaviorMSGQueue::MSG &m );

    static void findEnemy(GameObject& tg, int x, int y, int r, int team, GameObject*& out );
  };

#endif // ABILITY_H
