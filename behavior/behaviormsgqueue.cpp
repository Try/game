#include "behaviormsgqueue.h"
#include "game/world.h"

#include "algo/wayfindalgo.h"
#include "algo/algo.h"
#include "util/serialize.h"
#include "util/bytearrayserialize.h"

#include "game.h"

#include "recruterbehavior.h"

BehaviorMSGQueue::BehaviorMSGQueue( Game & owner )
                 :game(owner) {
  recvBuf.isRdy = false;
  data.reserve( 64 );
  recvBuf.data.reserve( 64 );
  }

bool BehaviorMSGQueue::message( Message msg,
                                int x, int y,
                                Modifers md ) {
  message(0, msg, x, y, md );
  return 1;
  }

void BehaviorMSGQueue::message(int pl, AbstractBehavior::Message msg) {
  message( pl, msg, 0, 0 );
  }

void BehaviorMSGQueue::message_st( int pl, AbstractBehavior::Message msg,
                                   size_t begin, size_t size) {
  message( pl, msg, 0, 0 );

  data.back().begin = begin;
  data.back().size  = size;
  }

void BehaviorMSGQueue::message( int pl,
                                Message msg, int x,
                                int y, Modifers md ) {
  MSG m;
  m.msg = msg;
  m.x   = x;
  m.y   = y;

  m.modifers = md;
  m.player   = pl;

  m.begin = 0;
  m.size  = 0;

  data.push_back(m);
  }

void BehaviorMSGQueue::message(int pl,
                                AbstractBehavior::Message msg,
                                int x, int y,
                                const std::string &str, size_t size,
                                AbstractBehavior::Modifers md) {
  message(pl, msg, x, y, md);
  data.back().str  = str;
  data.back().size = size;
  }

void BehaviorMSGQueue::tick(const Terrain &) {
  data.clear();
  }

void BehaviorMSGQueue::computeWay( World &w, const MSG& m ) {
  WayFindAlgo algo( w.terrain() );

  algo.fillClasrerMap( w.player(m.player).selected() );
  algo.findWay       ( w.player(m.player).selected(), m.x, m.y );

  remove_if( data, isMoveMSG );
  }

bool BehaviorMSGQueue::isMoveMSG(const BehaviorMSGQueue::MSG &m) {
  return m.msg==MoveGroup || m.msg==AtackMoveGroup;
  }

bool BehaviorMSGQueue::isSystemMSG(const BehaviorMSGQueue::MSG &m) {
  return SystemNullMessage <= m.msg && m.msg <= SystemLast;
  }

void BehaviorMSGQueue::unselect( World &w, int pl ) {
  //if( pl!=w.game.player().number() )
    //return;

  for( size_t i=0; i<w.objectsCount(); ++i )
    if( w.object(i).playerNum()==pl )
      w.object(i).unSelect();
  }

void BehaviorMSGQueue::select( World &w, int pl, size_t b, size_t size ) {
  //if( pl!=w.game.player().number() )
    //return;

  size += b;

  for( size_t i=b; i<size; ++i )
    if( w.object(i).playerNum()==pl )
      w.object(i).select();
  }

void BehaviorMSGQueue::tick( Game &game, World &w ) {
  sysMSG( game, w );

  for( size_t r=0; r<data.size(); ++r ){
    const MSG m = data[r];
    bool acepted = 0;

    Player &pl = w.game.player( m.player );
    for( size_t i=0; i<pl.selected().size(); ++i ){
      GameObject & obj = *pl.selected()[i];

      if( obj.isSelected() && obj.playerNum() ){
        bool a = 0;
        if( m.msg==Buy ){
          a = buyMsgRecv( game, w, obj, m );
          } else {
          a = obj.behavior.message( m.msg, m.x, m.y, m.modifers );
          }

        acepted |= a;

        if( (m.msg==Buy || m.msg==BuildAt) && a )
          break;
        }
      }

    if( m.msg==MoveGroup || m.msg==AtackMoveGroup ){
      computeWay(w,m);

      if( m.msg==MoveGroup ){
        w.emitHudAnim( "hud/move",
                       World::coordCast(m.x),
                       World::coordCast(m.y),
                       0.01 );
        }

      if( m.msg==AtackMoveGroup ){
        w.emitHudAnim( "hud/atack",
                       World::coordCast(m.x),
                       World::coordCast(m.y),
                       0.01 );
        }
      }
    }

  tick( w.terrain() );
  data.clear();
  }


void BehaviorMSGQueue::sysMSG( Game &game, World &w ){
  bool se = false;

  for( size_t r=0; r<data.size(); ++r ){
    const MSG m = data[r];

    if( m.msg == UnSelect ){
      unselect( w, m.player );
      se = 1;
      } else

    if( m.msg == SelectAdd ){
      select( w, m.player, m.begin, m.size );
      se = 1;
      } else

    if( m.msg == Select ){
      unselect( w, m.player );
      select( w, m.player, m.begin, m.size );
      se = 1;
      } else

    if( m.msg == EditAdd ){
      game.addEditorObject( m.str, m.player, m.x, m.y, m.size );
      } else

    if( m.msg == EditMove ){
      game.moveEditorObject( m.player, m.x, m.y );
      } else

    if( m.msg == EditRotate ){
      game.rotateEditorObject( m.player, m.x );
      } else

    if( m.msg == EditNext ){
      game.nextEditorObject( m.player );
      } else

    if( m.msg == EditDel ){
      game.delEditorObject( m.player );
      }

    }
  remove_if( data, isSystemMSG );
  }

bool BehaviorMSGQueue::buyMsgRecv( Game &game,
                                   World &w,
                                   GameObject& obj,
                                   const BehaviorMSGQueue::MSG &m) {
  std::vector<GameObject*> & v = obj.player().selected();

  size_t id = v.size();
  int t = 0;

  for( size_t i=0; i<v.size(); ++i ){
    GameObject & obj = *v[i];
    if( RecruterBehavior *r = obj.behavior.find<RecruterBehavior>() ){
      int t2 = r->qtime();
      if( id>=v.size() || t2<t ){
        id = i;
        t = t2;
        }
      }
    }

  if( id<v.size() )
    return v[id]->behavior.message( m.msg, m.str, m.modifers ); else
    return obj.behavior.message( m.msg, m.str, m.modifers );
  }

void BehaviorMSGQueue::serialize( Serialize &s ) {
  serialize(data,s);
  }

void BehaviorMSGQueue::serialize( std::vector<BehaviorMSGQueue::MSG> &data,
                                  Serialize &s ) {
  unsigned sz = data.size();
  s + sz;

  if( s.isEof() ){
    data.clear();
    return;
    }

  if( sz>1 ){
    int i;
    i = 0;
    }

  data.resize(sz);
  for( size_t i=0; i<data.size(); ++i ){
    MSG & m = data[i];

    unsigned msgCode = unsigned(m.msg);
    char b = '[', e = ']';
    s + b + msgCode + m.x + m.y + m.player + m.begin + m.size;

    s + m.str;
    s+e;

    m.msg = AbstractBehavior::Message(msgCode);
    }
  }

bool BehaviorMSGQueue::syncByNet( NetUser &usr ) {
  if( Lock(recvMutex, Lock::TryLock ) ){
    if( usr.isServer() ){
      for( size_t i=1; i<game.plCount(); ++i ){
        if( !game.player(i).isInSync() && game.player().number()!=int(i) )
          return false;
        }

      std::vector<char> v;
      ByteArraySerialize s(v, Serialize::Write);
      unsigned x = unsigned(pkServerAccept);
      s + x;
      int plN = game.player().number();
      s + plN;

      for( size_t i=0; i<recvBuf.data.size(); ++i )
        data.push_back( recvBuf.data[i] );
      recvBuf.data.clear();

      serialize( s );
      usr.sendMsg(v);

      for( size_t i=1; i<game.plCount(); ++i ){
        game.player(i).setSyncFlag(0);
        }
      return true;
      } else {//client
      if( recvBuf.isRdy ){
        std::vector<char> v;
        ByteArraySerialize s(v, Serialize::Write);
        unsigned x = unsigned(pkInGameSync);
        s + x;
        int plN = game.player().number();
        s + plN;
        serialize( s );

        usr.sendMsg(v);

        data = recvBuf.data;
        recvBuf.data.clear();
        recvBuf.isRdy = false;
        return true;
        }

      return false;
      }
    }

  return false;
  }

void BehaviorMSGQueue::onRecvSrv( const std::vector<char> &v ) {
  recvMutex.lock();

  RecvBuf buf;
  ByteArraySerialize s(v);

  unsigned pkgType = unsigned(pkInGameSync);
  s + pkgType;

  if( pkgType==pkInGameSync ){
    int plN = 0;
    s + plN;

    game.player(plN).setSyncFlag(1);

    serialize( buf.data, s );

    for( size_t i=0; i<buf.data.size(); ++i )
      recvBuf.data.push_back( buf.data[i] );
    //data.clear();
    }

  recvMutex.unlock();
  }

void BehaviorMSGQueue::onRecvClient( const std::vector<char> &v ) {
  RecvBuf buf;
  ByteArraySerialize s(v);

  unsigned pkgType = -1;
  s + pkgType;

  if( pkgType==pkServerAccept ){
    int plN = 0;
    s + plN;

    recvMutex.lock();

    serialize( buf.data, s );

    recvBuf.data  = buf.data;
    recvBuf.isRdy = true;

    recvMutex.unlock();
    }

  }

bool BehaviorMSGQueue::cmp( const BehaviorMSGQueue::MSG &m1,
                            const BehaviorMSGQueue::MSG &m2 ) {
  return m1.player < m2.player;
  }
