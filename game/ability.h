#ifndef ABILITY_H
#define ABILITY_H

#include "behavior/behaviormsgqueue.h"

class GameObject;
class World;
class Spell;

class Ability {
    Ability();

  public:
    static bool spell( Game &game, World &w,
                       GameObject &obj,
                       const BehaviorMSGQueue::MSG &m );

    static void autoCast( Game &game, World &w,
                          const std::string & spell,
                          GameObject &obj);
  private:
    static bool blink( Game &game, World &w,
                       GameObject &obj,
                       const BehaviorMSGQueue::MSG &m );

    static bool fireStrike( Game &game, World &w,
                            GameObject &obj,
                            const BehaviorMSGQueue::MSG &m );
    static bool fireStrike( World &w,
                            GameObject &obj,
                            GameObject &tg,
                            const Spell &s);
    static bool heal( Game &game, World &w,
                      GameObject &obj,
                      const BehaviorMSGQueue::MSG &m );
    static bool heal(World &w,
                      GameObject &u,
                      GameObject &tg , const Spell &s);

    static bool fireStorm( Game &game, World &w,
                           GameObject &obj,
                           const BehaviorMSGQueue::MSG &m );
    static bool fireStorm(World &w,
                           GameObject &u,
                           int tgX, int tgY,
                           const Spell &s);

    static bool forceField( Game &game, World &w,
                            GameObject &obj,
                            const BehaviorMSGQueue::MSG &m );
    static bool forceField( World &w,
                            GameObject &u,
                            int tgX, int tgY,
                            const Spell &s);

    static bool iceBolt( Game &game, World &w,
                         GameObject &obj,
                         const BehaviorMSGQueue::MSG &m );
    static bool iceBolt( World &w,
                         GameObject &obj,
                         GameObject &tg,
                         const Spell &s);

    static void findEnemy(GameObject& tg, int x, int y, int r, int team, GameObject*& out );
    static void findAlly(GameObject& tg, int x, int y, int r, GameObject &caster, GameObject*& out );
  };

#endif // ABILITY_H
