#ifndef MOVEBEHAVIOR_H
#define MOVEBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"

class Point;

class MoveBehavior : public AbstractBehavior {
  public:
    MoveBehavior( GameObject & obj,
                  Behavior::Closure & c );
    ~MoveBehavior();

    //bool message( Message msg, int x,
    //              int y, Modifers md = BehaviorEvent::NoModifer );
    void moveEvent( MoveEvent &m );
    void moveEvent( MoveSingleEvent  &m );
    void moveEvent( MineralMoveEvent &m );
    void stopEvent(StopEvent &m);
    void repositionEvent( RepositionEvent &m );
    void positionChangeEvent(PositionChangeEvent &);

    void tick( const Terrain & terrain );

    void setWay( const std::vector<Point> & v );
    int mask;
  private:
    GameObject & obj;
    Behavior::Closure & clos;

    //std::vector<Pos> wayPoints;
    int tx, ty, curentSpeed;
    int isLocked;
    bool isWayAcept, & isMWalk;
    std::vector<Point> way;

    void step( const Terrain & terrain, int sz, bool busyIgnoreFlag );
    bool nextPoint();

    GameObject* isCollide(int x, int y, int sz, const Terrain &terrain);

    void calcWayAndMove( int tx, int ty, const Terrain & terrain );
  };

#endif // MOVEBEHAVIOR_H
