#ifndef WARRIORBEHAVIOR_H
#define WARRIORBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"
#include "util/weakworldptr.h"

class WorkerBehavior;

class WarriorBehavior : public AbstractBehavior  {
  public:
    WarriorBehavior( GameObject & obj,
                     Behavior::Closure & c );
    ~WarriorBehavior();


    void tick( const Terrain &  );
    bool message(Message msg, int x, int y, Modifers md);
  private:
    GameObject & obj;
    WeakWorldPtr taget;

    void move(int, int);

    void damageTo( GameObject & obj );

    bool isAtk;
    int  dAtkTime;
  };

#endif // WARRIORBEHAVIOR_H
