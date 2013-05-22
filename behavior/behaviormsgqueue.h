#ifndef BEHAVIORMSGQUEUE_H
#define BEHAVIORMSGQUEUE_H

#include "behavior/abstractbehavior.h"
#include <vector>
#include <string>
#include <memory>

#include "threads/mutex.h"
#include "network/localserver.h"

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

    void message_st( int pl,
                     Message msg,
                     size_t id );

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

    void message( const std::vector<char> & t ){
      MSG m;
      m.msg   = UserEvent;
      m.udata = t;
      data.push_back(m);
      }

    template< class T >
    void message( const T & t ){
      MSG m;
      m.msg = UserEvent;
      m.udata.resize( sizeof(t) );
      memcpy( &m.udata[0], &t, sizeof(t) );

      data.push_back(m);
      }

    void tick( Game & game, World & w );
    void tick( const Terrain& );

    void onUnitRemove(size_t id);

    void serialize( Serialize & s );
    bool syncByNet( NetUser & usr );

    void onRecvSrv(const std::vector<char> &data );
    void onRecvClient(const std::vector<char> &data );
    void onNewClient(NetUser&, LocalServer::Client & lc );
    void onDelClient(NetUser&, LocalServer::Client & lc );

    Game & game;
  private:

    struct MSG{
      Message msg;
      int x, y, player;
      Modifers modifers;

      size_t begin, size;
      std::string str;
      std::vector<char> udata;
      };

    std::vector<MSG> data;

    enum PkgType{
      pkInGameSync,
      pkServerAccept,

      pkClientInit,

      pkInGameLoad,
      pkQuit
      };

    struct RecvBuf{
      std::vector<MSG> data;
      bool isRdy;
      };

    RecvBuf recvBuf;
    Mutex   recvMutex;

    struct Client{
      size_t pl;
      bool  isSync;
      void * pid;
      };

    std::vector<Client> clients;

    void serialize( std::vector<MSG>& data, Serialize & s );

    void computeWay( World &w, const MSG& m );
    static bool isMoveMSG( const MSG & m );
    static bool isSystemMSG( const MSG & m );


    void unselect( World & w, int pl );
    void select( World & w, int pl, size_t b, size_t size  );
    void sysMSG( Game & game, World & w );

    bool buyMsgRecv(Game & game, World & w, GameObject &obj, const MSG & m );
    bool spellMsgRecv(Game & game, World & w, GameObject &obj, const MSG & m );

  friend class Ability;
  };

#endif // BEHAVIORMSGQUEUE_H
