#ifndef BEHAVIORMSGQUEUE_H
#define BEHAVIORMSGQUEUE_H

#include "behavior/abstractbehavior.h"
#include <vector>
#include <string>

class World;
class Game;
class Serialize;

class BehaviorMSGQueue : public AbstractBehavior {
  public:
    BehaviorMSGQueue();

    bool message( Message msg,
                  int x, int y,
                  Modifers md = BehaviorEvent::NoModifer );

    void message( int pl,
                  Message msg );

    void message_st( int pl,
                     Message msg,
                     size_t begin,
                     size_t size );

    void message( int pl,
                  Message msg,
                  int x, int y,
                  Modifers md = BehaviorEvent::NoModifer );

    void message( int pl,
                  Message msg,
                  int x, int y,
                  const std::string& str,
                  size_t size,
                  Modifers md = BehaviorEvent::NoModifer );

    void tick( Game & game, World & w );
    void tick( const Terrain& );

    void serialize( Serialize & s );
  private:
    struct MSG{
      Message msg;
      int x, y, player;
      Modifers modifers;

      size_t begin, size;
      std::string str;
      };

    std::vector<MSG> data;

    void computeWay( World &w, const MSG& m );
    static bool isMoveMSG( const MSG & m );
    static bool isSystemMSG( const MSG & m );


    void unselect( World & w, int pl );
    void select( World & w, int pl, size_t b, size_t size  );
    void sysMSG( Game & game, World & w );

    bool buyMsgRecv(Game & game, World & w, GameObject &obj, const MSG & m );
  };

#endif // BEHAVIORMSGQUEUE_H
