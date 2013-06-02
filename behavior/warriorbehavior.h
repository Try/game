#ifndef WARRIORBEHAVIOR_H
#define WARRIORBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"
#include "util/weakworldptr.h"
#include "gui/inputhook.h"

class WorkerBehavior;

class WarriorBehavior : public AbstractBehavior  {
  public:
    WarriorBehavior( GameObject & obj,
                     Behavior::Closure & c );
    ~WarriorBehavior();


    void tick( const Terrain &  );
    bool message(Message msg, int x, int y, Modifers md);
    bool message(Message msg, size_t id, Modifers md);

    void aClick();
    static void mkDamage(GameObject& tg, int plOwner, int x, int y,
                          const ProtoObject::GameSpecific::Atack& v );
  private:
    GameObject & obj;
    WeakWorldPtr taget, mvTaget;

    InputHook hook;
    bool instaled;
    int& intentToHold;
    bool& isPatrul;

    void move(int, int);

    void damageTo( GameObject & obj );
    static void damageSplash(GameObject& tg ,
                              int team, int x, int y,
                              GameObject &nobj,
                              const ProtoObject::GameSpecific::Atack& v );

    bool isAtk; int lastX, lastY;
    int  dAtkTime, dRqTime;
    bool hasAtkTaget;

    bool isAClick; int acX, acY;

    bool &mvLock;
    int &lkX, &lkY;

    void positionChangeEvent(PositionChangeEvent &m);

    void tickAtack(bool ignoreVrange);

    static void lookOn(GameObject& tg ,
                       GameObject &obj,
                       WarriorBehavior *wb,
                       GameObject *&rawTg,
                       GameObject *&rawTgBuild,
                       int &dTg, int &dBuld);

    void takeTaget(GameObject *&out, GameObject * tg, int d );

    void mouseDown( Tempest::MouseEvent& e );
    void mouseUp  ( Tempest::MouseEvent& e );
    void onRemoveHook();

    void lockGround();
    void unlockGround();

    bool canShoot( GameObject & obj );
  };

#endif // WARRIORBEHAVIOR_H
