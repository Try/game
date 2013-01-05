#ifndef ABSTRACTBEHAVIOR_H
#define ABSTRACTBEHAVIOR_H

#include <string>

class Terrain;
class GameObject;

class Game;

class BehaviorEvent{
  public:
    BehaviorEvent();

    void accept();
    void ignore();
    bool isAccepted() const;

    enum Modifers{
      NoModifer = 0,
      Shift = 1,
      Ctrl  = 2
      } modif;
  private:
    bool acpt;
  };

struct MoveEvent : BehaviorEvent{
  int x,y;
  };

struct MoveSingleEvent : BehaviorEvent{
  int x,y;
  };

struct MineralMoveEvent : BehaviorEvent{
  int x,y;
  };

struct RepositionEvent : BehaviorEvent{
  int x,y;
  };

struct PositionChangeEvent : BehaviorEvent{
  int x,y;
  };

struct StopEvent : BehaviorEvent{
  };

struct CancelEvent : BehaviorEvent{
  };

class AbstractBehavior {
  public:
    virtual ~AbstractBehavior();

    enum Message {
      NoMessage,
      Cancel,
      Move,
      MoveSingle,
      MoveGroup,
      MineralMove,
      Hold,
      AtackMove,
      AtackMoveContinue,
      AtackMoveGroup,
      Atack,
      Buy,
      BuildAt,

      StopMove,

      /* signals */
      onPositionChange,

      /* request */
      Reposition,

      /* system */
      SystemNullMessage,
      UnSelect,
      Select,
      SelectAdd,

      EditAdd,
      EditMove,
      EditRotate,
      EditNext,
      EditDel,

      SystemLast
      };

    typedef BehaviorEvent::Modifers Modifers;

    virtual void atackMoveEvent    ( MoveEvent & m       );
    virtual void atackMoveEvent    ( MoveSingleEvent & m );
    virtual void atackContinueEvent( MoveSingleEvent & m );

    virtual void moveEvent  ( MoveEvent & m );
    virtual void moveEvent  ( MoveSingleEvent &m );
    virtual void moveEvent  ( MineralMoveEvent &m );
    virtual void stopEvent  ( StopEvent &m );
    virtual void cancelEvent( CancelEvent &m );

    virtual void repositionEvent( RepositionEvent &m );
    virtual void positionChangeEvent(PositionChangeEvent &m );

    virtual bool message( Message msg,
                          int x, int y,
                          Modifers md = BehaviorEvent::NoModifer );

    virtual bool message( Message msg,
                          const std::string &s,
                          Modifers md = BehaviorEvent::NoModifer );
    virtual void tick( const Terrain & terrain ) = 0;
  };

#endif // ABSTRACTBEHAVIOR_H
