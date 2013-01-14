#include "recruterbehavior.h"

#include "game/gameobject.h"
#include "game/world.h"

#include "algo/algo.h"
#include "game.h"

#include <iostream>

RecruterBehavior::RecruterBehavior( GameObject & o,
                                    Behavior::Closure &  )
                 :obj(o),
                   light( obj, obj.game().prototype("ycube") ),
                   flag ( obj, obj.game().prototype("flag" ) )  {
  time     = 0;
  queueLim = 0;

  flag.setRotation( 180 );
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

  flag.setPosition( rallyX, rallyY );
  flag.setViewPosition( World::coordCast(rallyX),
                        World::coordCast(rallyY),
                        0 );
  flag.setVisible( obj.isSelected() &&
                   obj.hasHostCtrl() &&
                   !(rallyX==obj.x() && rallyY==obj.y() ) );
  flag.tick();

  if( queue.size()==0 )
    return;

  if( time==0 ){
    if( queueLim || create( queue[0], terrain ) ){
      if( !queueLim ){
        for( size_t i=1; i<queue.size(); ++i )
          queue[i-1] = queue[i];
        queue.pop_back();
        }

      if( queue.size() ){
        const ProtoObject& p = obj.world().game.prototype( queue[0] );
        if( p.data.lim <= obj.player().lim() ){
          time = p.data.buildTime;
          obj.player().addLim( -p.data.lim );
          queueLim = false;
          } else {
          queueLim = true;
          }
        }
      } else {
      std::cout << "can't find placement for object" << std::endl;
      }
    } else {
    --time;
    }
  }

bool RecruterBehavior::message( AbstractBehavior::Message msg,
                                const std::string& cls,
                                AbstractBehavior::Modifers md ) {
  if( msg!=Buy )
    return 0;

  const ProtoObject& p = obj.world().game.prototype( cls );

  if( obj.player().canBuild( p ) ){
    obj.player().addGold( -p.data.gold );
    } else {
    return 0;
    }

  queue.push_back( cls );
  if( queue.size()==1 ){
    time = p.data.buildTime;
    obj.player().addLim( -p.data.lim );
    }

  return 1;
  }

bool RecruterBehavior::message( AbstractBehavior::Message msg,
                                int x, int y,
                                AbstractBehavior::Modifers /*md*/ ) {
  if( msg==Move || msg==onPositionChange ){
    rallyX = x;
    rallyY = y;
    }

  return 0;
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

int RecruterBehavior::qtime() {
  int r = time;
  for( size_t i=1; i<queue.size(); ++i ){
    const ProtoObject& p = obj.world().game.prototype( queue[i] );
    r += p.data.buildTime;
    }

  return r;
  }

bool RecruterBehavior::create(const std::string &s, const Terrain &terrain) {
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
