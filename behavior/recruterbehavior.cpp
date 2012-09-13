#include "recruterbehavior.h"

#include "game/gameobject.h"
#include "game/world.h"

#include "algo/algo.h"

#include <iostream>

RecruterBehavior::RecruterBehavior( GameObject & o,
                                    Behavior::Closure & c ):obj(o) {
  time = 0;
  }

RecruterBehavior::~RecruterBehavior() {

  }

void RecruterBehavior::tick( const Terrain &terrain ) {
  if( queue.size()==0 )
    return;

  if( time==0 ){
    if( create( queue[0], terrain ) ){
      for( size_t i=1; i<queue.size(); ++i )
        queue[i-1] = queue[i];
      queue.pop_back();

      if( queue.size() )
        time = 25;
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

  queue.push_back( cls );
  if( queue.size()==1 )
    time = 25;

  return 1;
  }

bool RecruterBehavior::message( AbstractBehavior::Message msg,
                                int x, int y,
                                AbstractBehavior::Modifers md) {
  if( msg==Move || msg==onPositionChange ){
    rallyX = x;
    rallyY = y;
    }

  return 0;
  }

bool RecruterBehavior::create(const std::string &s, const Terrain &terrain) {
  GameObject & tg = obj.world().addObject( s );

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

  tg.setPosition( x1, y1, obj.z() );
  tg.setPlayer( obj.playerNum() );

  tg.setViewDirection( -(obj.x()-x1), -(obj.y()-y1) );

  tg.behavior.message( MoveSingle, rallyX, rallyY );
  return true;
  }
