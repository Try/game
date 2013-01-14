#ifndef WORKERBEHAVIOR_H
#define WORKERBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"

#include "util/weakworldptr.h"

#include "game/gameobjectview.h"

class WorkerBehavior : public AbstractBehavior  {
  public:
    WorkerBehavior( GameObject & obj,
                    Behavior::Closure & c );
    ~WorkerBehavior();


    void tick( const Terrain & terrain );
    bool message( Message msg, int x, int y, Modifers md);
    bool message( Message msg,
                  size_t id,
                  Modifers md = BehaviorEvent::NoModifer );

  private:
    GameObject & obj;
    GameObjectView mineral;

    enum Mode{
      NoWork,
      ToMineral,
      Mining,
      ToCastle
      } mode;

    WeakWorldPtr res;
    WeakWorldPtr castle;
    int mtime;
    bool forceWalk;

    int bank;

    void toCastle();
    void toMineral();
    void move( int x, int y );

    void setMineral( const WeakWorldPtr& m );
  };

#endif // WORKERBEHAVIOR_H
