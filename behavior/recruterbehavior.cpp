#include "recruterbehavior.h"

#include "game/gameobject.h"
#include "game/world.h"
#include "game/upgrade.h"

#include "algo/algo.h"
#include "game.h"

#include <iostream>

RecruterBehavior::RecruterBehavior( GameObject & o,
                                    Behavior::Closure &  )
                 :obj(o),
                   light( obj, obj.game().prototype("ycube") ),
                   flag ( obj, obj.game().prototype("flag" ) )  {
  queueLim = 0;
  flag.setRotation( 180 );

  queue.resize(1);
  queue[0].time     = 0;
  queue[0].orders.reserve(6);
  }

RecruterBehavior::~RecruterBehavior() {

  }

void RecruterBehavior::tick( const Terrain &terrain ) {
  int x = obj.x(), y = obj.y();

  light.setPosition( x, y );
  light.setViewPosition( World::coordCast(x),
                         World::coordCast(y),
                         0 );
  light.setVisible( queue.size()>0 );
  light.tick();

  if( taget ){
    rallyX = obj.x();
    rallyY = obj.y();

    flag.setPosition( taget.value().x(), taget.value().y() );
    flag.setViewPosition( World::coordCast(taget.value().x()),
                          World::coordCast(taget.value().y()) );
    } else {
    flag.setPosition( rallyX, rallyY );
    flag.setViewPosition( World::coordCast(rallyX),
                          World::coordCast(rallyY) );
    }

  flag.setVisible( obj.isSelected() &&
                   obj.hasHostCtrl() &&
                   (!(rallyX==obj.x() && rallyY==obj.y() ) || taget ) );
  flag.tick();

  if( maxQueueSize()==0 )
    return;

  for( size_t r=0; r<queue.size(); ++r ){
    Queue &q = queue[r];

    if( queueLim || q.time==0 ){
      if( queueLim || create( q.orders[0], terrain ) ){
        if( !queueLim ){
          for( size_t i=1; i<q.orders.size(); ++i )
            q.orders[i-1] = q.orders[i];
          q.orders.pop_back();
          }

        if( q.orders.size() ){
          //const ProtoObject& p = obj.world().game.prototype( q.orders[0] );
          int lim = limOf( q.orders[0] );
          q.time  = q.orders[0].time;//p.data.buildTime;

          if( lim <= obj.player().lim() ){
            obj.player().addLim( -lim );
            queueLim = false;
            } else {
            queueLim = true;
            }
          }
        } else {
        std::cout << "can't find placement for object" << std::endl;
        }
      } else {
      --q.time;
      }
    }
  }

bool RecruterBehavior::message( AbstractBehavior::Message msg,
                                const std::string& cls,
                                AbstractBehavior::Modifers  ) {
  if( msg!=Upgrade && msg!=Buy )
    return 0;

  if( minQueueSize()>=6 )
    return 0;

  if( msg==Buy ){
    const ProtoObject& p = obj.world().game.prototype( cls );

    if( obj.player().canBuild( p ) ){
      obj.player().addGold( -p.data.gold );
      } else {
      return 0;
      }

    int t = minQtime();

    for( size_t i=0; i<queue.size(); ++i ){
      Queue &q = queue[i];
      if( qtime(q)==t ){
        q.addUnit( cls, p.data.buildTime );
        if( q.orders.size()==1 ){
          q.time = p.data.buildTime;
          obj.player().addLim( -p.data.lim );
          }

        return 1;
        }
      }
    }

  if( msg==Upgrade ){
    const ::Upgrade& u = obj.world().game.upgrade( cls );
    int lv = obj.player().gradeLv(u);

    if( obj.player().canBuild( u ) ){
      obj.player().addGold( -u.data[lv].gold );
      } else {
      return 0;
      }

    int t = minQtime();

    for( size_t i=0; i<queue.size(); ++i ){
      Queue &q = queue[i];
      if( qtime(q)==t ){
        q.addUpgrade( cls, u.data[lv].buildTime );
        if( q.orders.size()==1 ){
          q.time = u.data[lv].buildTime;
          }

        return 1;
        }
      }
    }

  return 0;
  }

bool RecruterBehavior::message( AbstractBehavior::Message msg,
                                int x, int y,
                                AbstractBehavior::Modifers /*md*/ ) {
  if( msg==Move || msg==onPositionChange ){
    rallyX = x;
    rallyY = y;
    taget  = WeakWorldPtr();
    }

  return 0;
  }

int RecruterBehavior::qtime() {
  int m = qtime(queue[0]);
  for( size_t i=1; i<queue.size(); ++i )
    m = std::max(m, qtime(queue[i]));

  return m;
  }

bool RecruterBehavior::message( AbstractBehavior::Message msg,
                                size_t id,
                                AbstractBehavior::Modifers /*md*/ ) {
  if( msg==ToUnit ){
    GameObject & m = obj.world().object(id);
    taget          = obj.world().objectWPtr(id);

    rallyX = m.x();
    rallyY = m.y();
    }

  return 0;
  }

int RecruterBehavior::qtime( const Queue & q ) const {
  int r = q.time;
  for( size_t i=1; i<q.orders.size(); ++i ){
    //const ProtoObject& p = obj.world().game.prototype( q.orders[i] );
    //r += p.data.buildTime;
    r += q.orders[i].time;
    }

  return r;
  }

int RecruterBehavior::ctime() const {
  int m = queue[0].time;
  for( size_t i=1; i<queue.size(); ++i )
    m = std::max(m, queue[i].time );

  return m;
  }

const std::vector<RecruterBehavior::Queue> &RecruterBehavior::orders() const {
  return queue;
  }

int RecruterBehavior::minQtime() {
  int m = qtime(queue[0]);
  for( size_t i=1; i<queue.size(); ++i )
    m = std::min(m, qtime(queue[i]));

  return m;
  }

bool RecruterBehavior::create( const Queue::Order &ord,
                               const Terrain &terrain ) {
  if( ord.type==Queue::Unit )
    return createUnit( ord.name, terrain );

  if( ord.type==Queue::Upgrade )
    return createUpgrade( ord.name );

  return 0;
  }

bool RecruterBehavior::createUpgrade( const std::string &s ) {
  Player &pl = obj.player();
  pl.mkGrade( obj.game().upgrade(s) );

  return 1;
  }

bool RecruterBehavior::createUnit( const std::string &s,
                                   const Terrain &terrain ) {
  int size = obj.getClass().data.size;
  int sq = Terrain::quadSize,
      x = obj.x()/sq,
      y = obj.y()/sq;

  int lx = x-size/2+size%2  -1,
      ly = y-size/2+size%2  -1,
      rx = lx+size          +2,
      ry = ly+size          +2,
      tgX = rallyX/sq,
      tgY = rallyY/sq;

  int l = -1, pX = 0, pY = 0;

  for( int i=lx; i<rx; ++i ){
    if( terrain.isEnable(i, ry) ){
      int d = abs(i-tgX) + abs(ry-tgY);

      if( l<0 || d<l ){
        l = d;
        pX = i;
        pY = ry;
        }
      }
    if( terrain.isEnable(i, ly) ){
      int d = abs(i-tgX) + abs(ly-tgY);

      if( l<0 || d<l ){
        l = d;
        pX = i;
        pY = ly;
        }
      }
    };

  for( int r=ly; r<ry; ++r ){
    if( terrain.isEnable(lx, r) ){
      int d = abs(lx-tgX) + abs(r-tgY);

      if( l<0 || d<l ){
        l = d;
        pX = lx;
        pY = r;
        }
      }
    if( terrain.isEnable(rx, r) ){
      int d = abs(rx-tgX) + abs(r-tgY);

      if( l<0 || d<l ){
        l = d;
        pX = rx;
        pY = r;
        }
      }
    };

  int x1 = pX *sq+sq/2,
      y1 = pY*sq+sq/2;

  if( l<0 )
    return false;

  GameObject & tg = obj.world().addObject( s );
  tg.setPosition( x1, y1, obj.z() );
  tg.setPlayer( obj.playerNum() );
  tg.player().addLim( tg.getClass().data.lim );

  tg.setViewDirection( -(obj.x()-x1), -(obj.y()-y1) );

  if( taget )
    tg.behavior.message( ToUnit, taget.id() ); else
    tg.behavior.message( MoveSingle, rallyX, rallyY );

  return true;
  }

size_t RecruterBehavior::minQueueSize() const {
  if( queue.size()==0 )
    return 0;

  size_t sz = queue[0].orders.size();
  for( size_t i=0; i<queue.size(); ++i )
    sz = std::min<size_t>( queue[i].orders.size(), sz );

  return sz;
  }

size_t RecruterBehavior::maxQueueSize() const {
  if( queue.size()==0 )
    return 0;

  size_t sz = queue[0].orders.size();
  for( size_t i=0; i<queue.size(); ++i )
    sz = std::max<size_t>( queue[i].orders.size(), sz );

  return sz;
  }

int RecruterBehavior::limOf(const RecruterBehavior::Queue::Order &ord) {
  if( ord.type==Queue::Unit ){
    const ProtoObject& p = obj.world().game.prototype( ord.name );
    return p.data.lim;
    }

  return 0;
  }
