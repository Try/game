#include "behaviormsgqueue.h"
#include "game/world.h"

#include "algo/wayfindalgo.h"
#include "algo/algo.h"
#include "util/serialize.h"
#include "util/bytearrayserialize.h"

#include "game.h"

#include "recruterbehavior.h"
#include "game/world.h"

#include "game/ability.h"

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
                                   size_t begin ) {
  message( pl, msg, 0, 0 );

  data.back().begin = begin;
  data.back().size  = 1;
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

void BehaviorMSGQueue::message( int pl,
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

void BehaviorMSGQueue::onUnitRemove( size_t id ) {
  for( size_t i=0; i<data.size(); ++i ){
    MSG & m = data[i];

    if( m.msg==SelectAdd || m.msg==Select ){
      if( m.begin+m.size > id )
        --m.size;

      if( m.begin > id )
        --m.begin;
      }

    if( m.msg==SpellCastU ){
      if( m.size==id )
        m.size = -1;

      if( m.size > id )
        --m.size;
      }
    }
  }

void BehaviorMSGQueue::computeWay( World &w, const MSG& m ) {
  int x = m.x,
      y = m.y;

  if( m.msg==MoveToUnitGroup ){
    GameObject& obj = w.object( m.begin );
    x = obj.x();
    y = obj.y();
    }
/*
  WayFindAlgo algo( w.terrain() );

  // algo.fillClasrerMap( w.player(m.player).selected() );
  algo.findWay       ( w.player(m.player).selected(), x, y );
  */
  std::vector<GameObject*>& s = w.player(m.player).selected();
  for( size_t i=0; i<s.size(); ++i )
    w.wayFind( x, y, s[i] );

  remove_if( data, isMoveMSG );
  }

bool BehaviorMSGQueue::isMoveMSG(const BehaviorMSGQueue::MSG &m) {
  return m.msg==MoveGroup ||
         m.msg==MoveToUnitGroup ||
         m.msg==AtackMoveGroup;
  }

bool BehaviorMSGQueue::isSystemMSG(const BehaviorMSGQueue::MSG &m) {
  return (SystemNullMessage <= m.msg && m.msg <= SystemLast) || m.msg==UserEvent;
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
    if( m.msg!=UserEvent ){
      for( size_t i=0; i<pl.selected().size(); ++i ){
        GameObject & obj = *pl.selected()[i];

        if( obj.isSelected() && obj.playerNum() ){
          bool a = 0;

          if( m.msg==Buy ){
            a = buyMsgRecv( game, w, obj, m );
            }
            else
          if( m.msg==SpellCast ){
            a = spellMsgRecv( game, w, obj, m );
            }
            else
          if( m.msg==ToUnit || m.msg==AtackToUnit ){
            GameObject& tg = w.object( m.begin );

            if( m.msg==AtackToUnit )
              tg.higlight( 25, GameObjectView::selAtk    ); else
              tg.higlight( 25, GameObjectView::selMoveTo );

            a = obj.behavior.message( m.msg, m.begin, m.modifers );
            } else {
            a = obj.behavior.message( m.msg, m.x, m.y, m.modifers );
            }

          acepted |= a;

          if( (m.msg==Buy || m.msg==BuildAt || m.msg==SpellCast) && a )
            break;
          }
        }
      }

    if( m.msg==MoveGroup ||
        m.msg==MoveToUnitGroup ||
        m.msg==AtackMoveGroup ){
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
  //bool se = false;

  for( size_t r=0; r<data.size(); ++r ){
    const MSG m = data[r];

    if( m.msg == UnSelect ){
      unselect( w, m.player );
      //se = 1;
      } else

    if( m.msg == SelectAdd ){
      select( w, m.player, m.begin, m.size );
      //se = 1;
      } else

    if( m.msg == Select ){
      unselect( w, m.player );
      select( w, m.player, m.begin, m.size );
      //se = 1;
      } else

    if( m.msg == EditAdd ){
      game.addEditorObject( m.str, m.player, m.x, m.y, 0, m.size );
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
      } else
    if( m.msg == UserEvent )
      game.scenario().customEvent( m.udata );
    }
  remove_if( data, isSystemMSG );
  }

bool BehaviorMSGQueue::buyMsgRecv( Game &,
                                   World &,
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

bool BehaviorMSGQueue::spellMsgRecv( Game &game, World &w,
                                     GameObject &obj,
                                     const BehaviorMSGQueue::MSG &m ) {
  return Ability::spell( game, w, obj, m );
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
    //int i;
    //i = 0;
    }

  data.resize(sz);
  for( size_t i=0; i<data.size(); ++i ){
    MSG & m = data[i];

    unsigned msgCode = unsigned(m.msg);
    char b = '[', e = ']';
    s + b + msgCode + m.x + m.y + m.player + m.begin + m.size;

    s + m.str;
    s + e;

    if( m.msg==UserEvent )
      s + m.udata;

    m.msg = AbstractBehavior::Message(msgCode);
    }
  }

bool BehaviorMSGQueue::syncByNet( NetUser &usr ) {
  if( Lock(recvMutex, Lock::TryLock ) ){
    if( usr.isServer() ){
      for( size_t i=0; i<clients.size(); ++i ){
        if( !clients[i].isSync &&
            game.player().number()!=int(clients[i].pl) )
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

      for( size_t i=0; i<clients.size(); ++i )
        clients[i].isSync = false;

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

    //game.player(plN).setSyncFlag(1);
    for( size_t i=0; i<clients.size(); ++i )
      if( int(clients[i].pl)==plN )
        clients[i].isSync = true;

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

  if( pkgType==pkClientInit ){
    size_t plN = 0;
    s + plN;
    game.setCurrectPlayer(plN);

    data.clear();
    recvBuf.data.clear();
    recvBuf.isRdy = true;
    }

  }

void BehaviorMSGQueue::onNewClient( NetUser &usr,
                                    LocalServer::Client &nc ) {
  recvMutex.lock();

  std::vector<char> v;
  ByteArraySerialize s(v, Serialize::Write);

  unsigned pkgType = unsigned(pkClientInit);
  s + pkgType;

  Client c;
  c.pl     = game.plCount();
  c.isSync = false;
  c.pid    = &nc;

  for( int i=2/*null player + server player*/; i<game.plCount(); ++i ){
    bool freedSlot = true;
    for( size_t r=0; r<clients.size(); ++r ){
      if( int(clients[r].pl)==i )
        freedSlot = false;
      }

    if( freedSlot ){
      c.pl = i;
      break;
      }
    }

  s+c.pl;
  if( int(c.pl)==game.plCount() ){
    game.player(c.pl);
    }

  clients.push_back(c);
  usr.sendMsg(v, nc);

  recvMutex.unlock();
  }

void BehaviorMSGQueue::onDelClient (NetUser &, NetUser::Client &c) {
  recvMutex.lock();

  for( size_t i=0; i<clients.size(); ++i ){
    if( clients[i].pid == &c ){
      clients[i] = clients.back();
      clients.pop_back();
      recvMutex.unlock();
      return;
      }
    }

  recvMutex.unlock();
  }

