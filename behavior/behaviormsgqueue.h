#ifndef BEHAVIORMSGQUEUE_H
#define BEHAVIORMSGQUEUE_H

#include "behavior/abstractbehavior.h"
#include <vector>
#include <string>
#include <memory>

#include "threads/mutex.h"

class World;
class Game;
class Serialize;
class NetUser;

class BehaviorMSGQueue : public AbstractBehavior {
  public:
    BehaviorMSGQueue( Game & owner );

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
    bool syncByNet( NetUser & usr );

    void onRecvSrv(const std::vector<char> &data );
    void onRecvClient(const std::vector<char> &data );
  private:
    Game & game;

    struct MSG{
      Message msg;
      int x, y, player;
      Modifers modifers;

      size_t begin, size;
      std::string str;
      };

    std::vector<MSG> data;

    static bool cmp( const MSG& m1, const MSG& m2 );

    enum PkgType{
      pkInGameSync,
      pkServerAccept,
      pkInGameLoad,
      pkQuit
      };

    struct RecvBuf{
      std::vector<MSG> data;
      bool isRdy;
      };

    RecvBuf recvBuf;
    Mutex   recvMutex;

    void serialize( std::vector<MSG>& data, Serialize & s );

    void computeWay( World &w, const MSG& m );
    static bool isMoveMSG( const MSG & m );
    static bool isSystemMSG( const MSG & m );


    void unselect( World & w, int pl );
    void select( World & w, int pl, size_t b, size_t size  );
    void sysMSG( Game & game, World & w );

    bool buyMsgRecv(Game & game, World & w, GameObject &obj, const MSG & m );
  };

#endif // BEHAVIORMSGQUEUE_H
