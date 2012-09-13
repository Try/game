#ifndef ABSTRACTBEHAVIOR_H
#define ABSTRACTBEHAVIOR_H

#include <string>

class Terrain;
class GameObject;

class Game;

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
      Atack,
      Buy,
      BuildAt,

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

    enum Modifers{
      NoModifer = 0,
      Shift = 1,
      Ctrl  = 2
      };

    virtual bool message( Message msg,
                          int x, int y,
                          Modifers md= NoModifer );

    virtual bool message( Message msg,
                          const std::string &s,
                          Modifers md= NoModifer );
    virtual void tick( const Terrain & terrain ) = 0;
  };

#endif // ABSTRACTBEHAVIOR_H
