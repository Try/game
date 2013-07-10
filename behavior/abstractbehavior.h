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

struct MoveToUnitEvent : BehaviorEvent{
  size_t id;
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

struct PatrulEvent : BehaviorEvent{
  int  x,  y;
  };

struct StopEvent : BehaviorEvent{
  };

struct HoldEvent : BehaviorEvent{
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
      ToUnit,
      MoveSingle,

      MoveGroup,
      MoveToUnitGroup,

      MineralMove,
      Hold,
      AtackToUnit,
      AtackMove,
      AtackMoveContinue,
      AtackMoveGroup,
      Atack,
      Buy,
      Upgrade,
      BuildAt,

      Patrul,

      StopMove,

      SpellCast,
      SpellCastU,

      /* custom */
      UserEvent,

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
    virtual void moveEvent  ( MoveToUnitEvent & m );
    virtual void moveEvent  ( PatrulEvent &m );

    virtual void moveEvent  ( MoveSingleEvent &m );
    virtual void moveEvent  ( MineralMoveEvent &m );
    virtual void stopEvent  ( StopEvent &m );
    virtual void holdEvent  ( HoldEvent &m );
    virtual void cancelEvent( CancelEvent &m );

    virtual void repositionEvent( RepositionEvent &m );
    virtual void positionChangeEvent(PositionChangeEvent &m );

    virtual bool message( Message msg,
                          int x, int y,
                          Modifers md = BehaviorEvent::NoModifer );

    virtual bool message( Message msg,
                          size_t id,
                          Modifers md = BehaviorEvent::NoModifer );

    virtual bool message( Message msg,
                          const std::string &s,
                          Modifers md = BehaviorEvent::NoModifer );
    virtual void tick( const Terrain & terrain ) = 0;
  };

#endif // ABSTRACTBEHAVIOR_H
