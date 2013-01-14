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
  private:
    GameObject & obj;
    WeakWorldPtr taget, mvTaget;

    InputHook hook;
    bool instaled;

    void move(int, int);

    void damageTo( GameObject & obj );

    bool isAtk; int lastX, lastY;
    int  dAtkTime;

    bool isAClick; int acX, acY;

    bool mvLock;
    int lkX, lkY;

    void positionChangeEvent(PositionChangeEvent &m);

    void tickAtack(bool ignoreVrange);

    static void lookOn(GameObject& tg ,
                       GameObject &obj,
                       WarriorBehavior *wb,
                       GameObject *&rawTg,
                       GameObject *&rawTgBuild,
                       int &dTg, int &dBuld);

    void takeTaget(GameObject *&out, GameObject * tg, int d );

    void mouseDown(MyWidget::MouseEvent &e);
    void mouseUp  ( MyWidget::MouseEvent& e );
    void onRemoveHook();
  };

#endif // WARRIORBEHAVIOR_H
