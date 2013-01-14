#include "workerbehavior.h"

#include "game/gameobject.h"
#include "game/world.h"
#include "resourcebehavior.h"
#include "warehousebehavior.h"

#include "game/player.h"
#include "game.h"

#include <cmath>

WorkerBehavior::WorkerBehavior( GameObject & object,
                                Behavior::Closure &  )
  :obj(object), mineral( obj, obj.game().prototype("crystal") ) {
  mode  = NoWork;
  mtime = 0;
  forceWalk = 0;

  mineral.setViewSize(0.4, 0.4, 0.4);

  bank = 0;
  }

WorkerBehavior::~WorkerBehavior() {
  setMineral( WeakWorldPtr() );
  }

void WorkerBehavior::tick( const Terrain &/*terrain*/ ) {
  int x = obj.x(), y = obj.y();

  int vx = 0, vy = 0;
  vx = 100*cos( obj.rAngle() );
  vy = 100*sin( obj.rAngle() );

  mineral.setPosition( x, y );
  mineral.setViewPosition( World::coordCast(x+vx),
                           World::coordCast(y+vy),
                           0 );
  mineral.setViewDirection(vx,vy);
  mineral.setVisible( bank>0 );
  mineral.tick();

  if( mode==ToMineral && !res && (!forceWalk || !obj.isOnMove()) ){
    toMineral();
    }

  if( mode==ToMineral && res ){
    GameObject & re = res.value();

    if( !obj.isOnMove() ){
      toMineral();
      }

    int d = re.distanceQL(x, y);
    if( d < 1 ){
      if( re.behavior.find<ResourceBehavior>()->isBusy==this ){
        mode  = Mining;
        mtime = 50;
        } else
        toMineral();

      return;
      }
    }

  if( mode==ToCastle && castle ){
    GameObject & cas = castle.value();

    int d = cas.distanceQL(x, y);
    int s = cas.getClass().data.size/2+1;

    if( cas.getClass().data.size%2 )
      ++s;

    if( d <= s ){
      //mode = ToMineral;
      obj.player().addGold( bank );
      bank = 0;
      toMineral();
      return;
      } else {
      if( !obj.isOnMove() )
        toCastle();
      }
    }


  if( mode==Mining && mtime>=0 ){
    if( bank==0 && mtime>0 && res ){
      assert( res.value().behavior.find<ResourceBehavior>()->isBusy==this );
      --mtime;
      } else {
      bank = 5;
      mode  = ToCastle;
      toCastle();
      }
    }
  }

bool WorkerBehavior::message( AbstractBehavior::Message msg,
                              int /*x*/, int /*y*/,
                              AbstractBehavior::Modifers /*md*/ ) {
  if( msg!=Move && msg!=MoveSingle )
    return 0;

  setMineral( WeakWorldPtr() );
  mode = NoWork;
  return 0;
  }

bool WorkerBehavior::message( Message msg,
                              size_t id,
                              AbstractBehavior::Modifers /*md*/ ) {
  if( msg!=ToUnit )
    return 0;

  setMineral( WeakWorldPtr() );

  World & w = obj.world();
  if( w.object(id).behavior.find<ResourceBehavior>() ){
    GameObject & m = w.object(id);

    move( m.x(), m.y() );
    forceWalk = 1;
    mode = ToMineral;
    return 1;
    }

  return 0;
  }

void WorkerBehavior::toCastle() {
  setMineral( WeakWorldPtr() );

  int x = obj.x(), y = obj.y();
  World & w = obj.world();

  size_t id = 0;
  int len = -1;

  for( size_t i=0; i<w.warehouse().size(); ++i )
    if( w.warehouse()[i]->playerNum() == obj.playerNum() &&
        w.warehouse()[i]->behavior.find<WarehouseBehavior>() ){
      GameObject & m = *w.warehouse()[i];
      int nd = m.distanceSQ(x, y);
      if( nd < len || len<0 ){
        id = i;
        len = nd;
        }
      }

  if( len>=0 ){
    GameObject & m = *w.warehouse()[id];
    move( m.x(), m.y() );
    castle = w.objectWPtr( &m );
    mode = ToCastle;
    }
  }

void WorkerBehavior::toMineral() {
  const int maxLen = 10;
  forceWalk = 0;

  int x = obj.x(), y = obj.y();
  World & w = obj.world();

  size_t id = 0;
  int len = -1;

  for( size_t i=0; i<w.resouce().size(); ++i )
    if( ResourceBehavior *b = w.resouce()[i]->behavior.find<ResourceBehavior>() ){
      GameObject & m = *w.resouce()[i];
      int nd = m.distanceQL(x, y);
      if( (b->isBusy==0 || b->isBusy==this ) &&
          (nd < len || len<0) ){
        id = i;
        len = nd;
        }
      }

  if( len<0 || len>=maxLen ){
    for( size_t i=0; i<w.resouce().size(); ++i )
      if( w.resouce()[i]->behavior.find<ResourceBehavior>() ){
        GameObject & m = *w.resouce()[i];
        int nd = m.distanceQL(x, y);
        if( (nd < len || len<0) ){
          id = i;
          len = nd;
          }
        }
    }

  if( len>=0 && len<maxLen ){
    GameObject & m = *w.resouce()[id];
    move( m.x(), m.y() );

    if( !m.behavior.find<ResourceBehavior>()->isBusy )
      setMineral( w.objectWPtr( &m ) );
    mode = ToMineral;
    }
  }

void WorkerBehavior::move(int x, int y) {
  int qs = Terrain::quadSize;

  x /= qs;
  y /= qs;
  obj.behavior.message( MineralMove, x*qs + qs/2, y*qs + qs/2 );
  }

void WorkerBehavior::setMineral( const WeakWorldPtr &m ) {
  if( res )
    res.value().behavior.find<ResourceBehavior>()->isBusy = 0;

  if( m ){
    WeakWorldPtr x = m;
    assert( x.value().behavior.find<ResourceBehavior>()->isBusy==0 );
    }

  res = m;

  if( res ){
    res.value().behavior.find<ResourceBehavior>()->isBusy = this;
    }
  }
