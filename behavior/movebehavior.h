#ifndef MOVEBEHAVIOR_H
#define MOVEBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"
#include "gui/inputhook.h"

class Point;

class MoveBehavior : public AbstractBehavior {
  public:
    MoveBehavior( GameObject & obj,
                  Behavior::Closure & c );
    ~MoveBehavior();

    //bool message( Message msg, int x,
    //              int y, Modifers md = BehaviorEvent::NoModifer );
    void atackMoveEvent(MoveSingleEvent &m);
    void atackContinueEvent( MoveSingleEvent & m );

    void moveEvent( MoveEvent &m );
    void moveEvent( MoveSingleEvent  &m );
    void moveEvent( MineralMoveEvent &m );
    void stopEvent(StopEvent &m);
    void cancelEvent(CancelEvent &m);
    void repositionEvent( RepositionEvent &m );
    void positionChangeEvent(PositionChangeEvent &);

    void tick( const Terrain & terrain );

    void setWay( const std::vector<Point> & v );
    bool isSameDirection( const MoveBehavior& other );

    void setupMoveHook();

    static bool isCloseEnough(int x1, int y1, int x2, int y2 , int unitSize);
private:
    GameObject & obj;
    Behavior::Closure & clos;

    InputHook hook;
    bool instaled;

    //std::vector<Pos> wayPoints;
    int tx, ty, curentSpeed;
    bool isWayAcept, & isMWalk;
    std::vector<Point> way;

    void step( const Terrain & terrain, int sz, bool busyIgnoreFlag );
    bool nextPoint();

    void calcWayAndMove( int tx, int ty, const Terrain & terrain );

    void mouseDown(MyWidget::MouseEvent &e);
    void mouseUp  ( MyWidget::MouseEvent& e );
    void onRemoveHook();
  };

#endif // MOVEBEHAVIOR_H
