#ifndef WORKERBEHAVIOR_H
#define WORKERBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"

#include "util/weakworldptr.h"

class WorkerBehavior : public AbstractBehavior  {
  public:
    WorkerBehavior( GameObject & obj,
                    Behavior::Closure & c );
    ~WorkerBehavior();


    void tick( const Terrain & terrain );
    bool message( Message msg, int x, int y, Modifers md);

  private:
    GameObject & obj;

    enum Mode{
      NoWork,
      ToMineral,
      Mining,
      ToCastle
      } mode;

    WeakWorldPtr res;
    WeakWorldPtr castle;
    int mtime;
    bool foceWalk;

    void toCastle();
    void toMineral();
    void move( int x, int y );

    void setMineral( const WeakWorldPtr& m );
  };

#endif // WORKERBEHAVIOR_H
