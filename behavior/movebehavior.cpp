#include "movebehavior.h"

#include "game/gameobject.h"
#include "util/math.h"
#include "game/world.h"

#include "algo/algo.h"
#include "algo/wayfindalgo.h"

#include "game.h"

MoveBehavior::MoveBehavior( GameObject &object,
                            Behavior::Closure & c )
  :obj(object), clos(c), isMWalk(c.isMineralMove) {
  tx = 0;
  ty = 0;
  isLocked = 0;
  isWayAcept = 0;
  isMWalk    = 0;

  curentSpeed = 0;

  clos.isMoviable = true;
  mask = -1;
  }

MoveBehavior::~MoveBehavior() {
  clos.isMoviable  = false;
  clos.isOnMove    = false;
  clos.isReposMove = false;
  }

bool MoveBehavior::message( Message msg, int x, int y, Modifers md ) {
  clos.isReposMove = 0;

  if( msg==Move ){/*
    tx = x;
    ty = y;

    clos.isOnMove = true;*/
    isWayAcept = 1;
    isMWalk    = 0;
    obj.world().game.message( obj.playerNum(), MoveGroup, x, y, md);
    }

  if( msg==MoveSingle ){
    calcWayAndMove( x, y, obj.world().terrain() );
    }

  if( msg==MineralMove ){
    way.clear();
    isMWalk = 1;

    tx = x;
    ty = y;
    mask = -1;
    isLocked = 0;

    clos.isOnMove    = true;
    clos.isReposMove = false;

    return 1;
    }

  if( msg==Reposition && !clos.isOnMove && !isMWalk ){
    tx = x;
    ty = y;

    clos.isReposMove = true;
    clos.isOnMove    = true;

    return 1;
    }

  if( msg==onPositionChange || msg==Cancel || msg==Hold ){
    tx = obj.x();
    ty = obj.y();
    isLocked = 0;
    }

  return 0;
  }

GameObject *MoveBehavior::isCollide( int x, int y,
                                     int sz,
                                     const Terrain &terrain ) {
  int qs = Terrain::quadSize;

  GameObject * a = terrain.unitAt(x, y);

  if( a && a!=&this->obj ){
    int dx = obj.x()/qs-x,
        dy = obj.y()/qs-y;

    int sz1 = a ->getClass().data.size,
        sz2 = obj.getClass().data.size;

    if((dx*dx + dy*dy) < (sz1*sz1 + sz2*sz2))
      return a;
    }

  return 0;
  }

void MoveBehavior::calcWayAndMove(int tx, int ty, const Terrain & terrain ) {
  WayFindAlgo algo(terrain);
  algo.findWay( obj,
                obj.x()/Terrain::quadSize,
                obj.y()/Terrain::quadSize,
                tx/Terrain::quadSize,
                ty/Terrain::quadSize );
  isWayAcept = 1;
  setWay( algo.way );

  /*
  if( way.size() ){
    way.push_back( Point() );
    way.back().x = tx;
    way.back().y = ty;
    }*/
  }

void MoveBehavior::tick(const Terrain &terrain) {
  int qs = Terrain::quadSize;
  int x = obj.x()/qs,
      y = obj.y()/qs;

  int sz = obj.getClass().data.size;

  int bm = 1;
  bm = terrain.busyAt(x,y, sz);

  if( terrain.isEnableQuad( x, y, sz ) && ( clos.isReposMove || bm<=1 ) ){
    if( clos.isOnMove || clos.isReposMove ){
      step(terrain, sz, false);
      }
    } else {
    bool ch = false;

    if( terrain.unitAt(x,y)!=&obj ){
      auto p = terrain.nearestEnable( x, y, sz );
      int nx = p.first,
          ny = p.second;

      while( nx!=x || ny!=y ){
        int lx = nx, ly = ny;
        if( abs(nx-x) > abs(ny-y) ){
          if( nx>x )
            nx -= 1; else
            nx += 1;
          } else {
          if( ny>y )
            ny -= 1; else
            ny += 1;
          }

        GameObject * obj = terrain.unitAt(nx, ny);
        if( obj  ){
          MoveBehavior * b = obj->behavior.find<MoveBehavior>();
          if( b && !b->clos.isReposMove/* &&
              (mask!=b->mask || mask<0)*/ ){
            b->message( Reposition, lx*qs + qs/2, ly*qs + qs/2 );
            if( !b->clos.isOnMove )
              b->step( terrain, sz, false );
            }
          ch = (b!=0 &&(mask!=b->mask || mask<0) );
          }
        }
      }

    if( !ch || isMWalk )
      step(terrain, sz, true);
    }

  }

void MoveBehavior::step(const Terrain &terrain, int sz, bool busyIgnoreFlag ) {
  int acseleration = 2;

  if( curentSpeed+acseleration < obj.getClass().data.speed )
    curentSpeed+=acseleration; else
    curentSpeed = obj.getClass().data.speed;

  int l = Math::distance( obj.x(), obj.y(),
                          tx, ty )/curentSpeed;

  if( l>2 ){
    int x = obj.x()+(tx-obj.x())/l,
        y = obj.y()+(ty-obj.y())/l;

    float wx = x/Terrain::quadSizef,
          wy = y/Terrain::quadSizef;

    int iwx = x/Terrain::quadSize,
        iwy = y/Terrain::quadSize;

    int pwx = obj.x()/Terrain::quadSize,
        pwy = obj.y()/Terrain::quadSize;

    GameObject * u = terrain.unitAt(iwx,iwy);
    MoveBehavior * b = 0;

    int dot = 1;
    if( u ){
      b = u->behavior.find<MoveBehavior>();
      if( b )
        dot = (obj.x()-tx)*(u->x() - b->tx) +
              (obj.y()-ty)*(u->y() - b->ty);
      }
    int lockMax = 8*terrain.busyAt(iwx, iwy, sz);

    bool isBusy = ( isMWalk ||
                    u == 0 ||
                    b->isMWalk ||
                    u == &obj  ||
                    !u->isOnMove() ||
                    ( b && (dot>0 || isLocked > 5*lockMax) && isLocked > lockMax) ||
                    clos.isReposMove )
                  || busyIgnoreFlag;

    if( (terrain.isEnable( iwx, iwy) ) ||
        !terrain.isEnable( pwx, pwy) ){
      if( isBusy ){
        int ltx = tx, lty = ty;

        obj.setPosition( x, y, terrain.heightAt(wx,wy) );
        obj.setViewDirection( ltx-obj.x(), lty-obj.y() );

        tx = ltx;
        ty = lty;
        isLocked = 0;
        } else {
        isLocked += 1;
        }
      } else {
      if( way.size() ){
        tx = way[0].x;
        ty = way[0].y;
        }

      if( !clos.isReposMove/* && !isMWalk*/ ){
        calcWayAndMove( tx, ty, terrain );
        }
      }

    } else {
    if( !nextPoint() ){
      clos.isOnMove     = false;
      clos.isReposMove  = false;
      //isMWalk = 0;
      mask = -1;
      curentSpeed = 0;
      }
    }

  }

bool MoveBehavior::nextPoint() {
  if( way.size()==0 ){
    tx = obj.x();
    ty = obj.y();

    clos.isReposMove = true;
    clos.isOnMove    = true;
    return false;
    }

  Point b = way.back();
  way.pop_back();

  tx = b.x;
  ty = b.y;

  clos.isOnMove     = true;
  clos.isReposMove  = false;

  return true;
  }

void MoveBehavior::setWay( const std::vector<Point> &v ) {
  if( isWayAcept==0 )
    return;
  isWayAcept = 0;

  mask = -1;
  way  = v;

  if( v.size()==0 ){
    tx = obj.x();
    ty = obj.y();

    clos.isReposMove = false;
    clos.isOnMove    = false;
    return;
    }

  int qs = Terrain::quadSize, hqs = qs/2;

  int dx = v.back().x*qs + hqs - obj.x(),
      dy = v.back().y*qs + hqs - obj.y();

  if( !obj.world().terrain().isEnableQuad( obj.x()/qs, obj.y()/qs, 2) ){
    dx = 0;
    dy = 0;
    }

  //way.pop_back();

  for( size_t i=0; i<way.size(); ++i ){
    way[i].x = way[i].x*qs + hqs - dx;
    way[i].y = way[i].y*qs + hqs - dy;
    }
/*
  way.push_back( way.back() );
  way.back().x = tx;
  way.back().y = ty;*/

  nextPoint();
  }
