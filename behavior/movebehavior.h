#ifndef MOVEBEHAVIOR_H
#define MOVEBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"
#include "gui/inputhook.h"
#include "util/weakworldptr.h"

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
    void moveEvent( MoveToUnitEvent & m );
    void moveEvent( MoveSingleEvent  &m );
    void moveEvent( MineralMoveEvent &m );
    void stopEvent(StopEvent &m);
    void cancelEvent(CancelEvent &m);
    void repositionEvent( RepositionEvent &m );
    void positionChangeEvent(PositionChangeEvent &);

    void tick( const Terrain & terrain );

    void setWay( const std::vector<Point> & v );
    void takeWay( const MoveBehavior & v );

    bool isSameDirection( const MoveBehavior& other );

    void setupMoveHook();

    static bool isCloseEnough(int x1, int y1, int x2, int y2 , int unitSize);

    void updatePos( const Terrain &t );
  private:
    GameObject & obj;
    Behavior::Closure & clos;

    InputHook hook;
    bool instaled;
    int timer;

    //std::vector<Pos> wayPoints;
    int tx, ty, curentSpeed;
    int intentPos[2];

    WeakWorldPtr taget;

    bool isWayAcept, & isMWalk;
    std::vector<Point> way;

    void step(const Terrain & terrain);
    bool nextPoint();

    void calcWayAndMove( int tx, int ty, const Terrain & terrain );

    void mouseDown(MyWidget::MouseEvent &e);
    void mouseUp  ( MyWidget::MouseEvent& e );
    void onRemoveHook();
  };

#endif // MOVEBEHAVIOR_H
