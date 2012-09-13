#include "behaviormsgqueue.h"
#include "game/world.h"

#include "algo/wayfindalgo.h"
#include "algo/algo.h"
#include "util/serialize.h"
#include "game.h"

BehaviorMSGQueue::BehaviorMSGQueue() {
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

void BehaviorMSGQueue::message( int pl,
                                AbstractBehavior::Message msg,
                                int x, int y,
                                const std::string &str,
                                AbstractBehavior::Modifers md) {
  message(pl, msg, x, y, md);
  data.back().str = str;
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
  return m.msg==MoveGroup;
  }

bool BehaviorMSGQueue::isSystemMSG(const BehaviorMSGQueue::MSG &m) {
  return SystemNullMessage <= m.msg && m.msg <= SystemLast;
  }

void BehaviorMSGQueue::unselect( World &w, int pl ) {
  for( size_t i=0; i<w.objectsCount(); ++i )
    if( w.object(i).playerNum()==pl )
      w.object(i).unSelect();
  }

void BehaviorMSGQueue::select( World &w, int pl, size_t b, size_t size ) {
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

    for( size_t i=0; i<w.objectsCount(); ++i ){
      GameObject & obj = w.object(i);

      if( obj.isSelected() ){
        bool a = 0;
        if( m.msg==Buy ){
          a = obj.behavior.message( m.msg, m.str, m.modifers );
          } else {
          a = obj.behavior.message( m.msg, m.x, m.y, m.modifers );
          }

        acepted |= a;

        if( (m.msg==Buy || m.msg==BuildAt) && a )
          break;
        }
      }
/*
    if( m.msg==Buy ){
      GameObject & obj = w.addObjectEnv( m.str );
      obj.setPosition( m.x, m.y, 10 );
      }*/

    if( m.msg==MoveGroup ){
      computeWay(w,m);
      }
    }

  tick( w.terrain() );
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
      game.addEditorObject( m.str, m.player, m.x, m.y );
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

void BehaviorMSGQueue::serialize( Serialize &s ) {
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
