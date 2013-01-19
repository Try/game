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

  timer = 0;

  isWayAcept = 0;
  isMWalk    = 0;

  curentSpeed = 0;

  clos.colisionDisp[0] = 0;
  clos.colisionDisp[1] = 0;

  clos.isMoviable = true;

  instaled = 0;
  hook.mouseDown.bind( *this, &MoveBehavior::mouseDown    );
  hook.mouseUp  .bind( *this, &MoveBehavior::mouseUp      );
  hook.onRemove .bind( *this, &MoveBehavior::onRemoveHook );
  }

MoveBehavior::~MoveBehavior() {
  clos.isMoviable  = false;
  clos.isOnMove    = false;
  clos.isReposMove = false;

  if( instaled )
    obj.game().removeHook( &hook );
  }

void MoveBehavior::atackMoveEvent( MoveSingleEvent &m ) {
  taget = WeakWorldPtr();

  clos.isReposMove = 0;
  clos.isOnMove    = true;

  isWayAcept = 1;
  isMWalk    = 0;
  obj.world().game.message( obj.playerNum(),
                            AtackMoveGroup, m.x, m.y, m.modif );
  }

void MoveBehavior::atackContinueEvent(MoveSingleEvent &m) {
  moveEvent(m);
  }

void MoveBehavior::moveEvent( MoveEvent &m ) {
  clos.isReposMove = 0;
  taget = WeakWorldPtr();

  isWayAcept = 1;
  isMWalk    = 0;
  obj.world().game.message( obj.playerNum(), MoveGroup, m.x, m.y, m.modif );
  }

void MoveBehavior::moveEvent(MoveToUnitEvent &m) {
  clos.isReposMove = 0;

  taget = obj.world().objectWPtr( m.id );

  isWayAcept = 1;
  isMWalk    = 0;
  obj.world().game.message( obj.playerNum(),
                            MoveToUnitGroup,
                            m.id,
                            m.modif );
  }

void MoveBehavior::moveEvent( MoveSingleEvent &m ) {
  taget = WeakWorldPtr();

  clos.isOnMove    = true;
  clos.isReposMove = 0;
  isMWalk    = 0;

  calcWayAndMove( m.x, m.y, obj.world().terrain() );
  }

void MoveBehavior::moveEvent( MineralMoveEvent &m ) {
  taget = WeakWorldPtr();

  clos.isReposMove = 0;

  way.clear();
  isMWalk = 1;

  tx = m.x;
  ty = m.y;

  clos.isOnMove    = true;
  clos.isReposMove = false;
  }

void MoveBehavior::stopEvent(StopEvent &) {
  clos.isReposMove = 0;

  way.clear();
  taget = WeakWorldPtr();

  isMWalk = 0;

  tx = obj.x();
  ty = obj.y();

  clos.isOnMove    = false;
  clos.isReposMove = false;
  }

void MoveBehavior::cancelEvent(CancelEvent &) {
  clos.isReposMove = 0;

  way.clear();
  taget = WeakWorldPtr();

  isMWalk = 0;

  tx = obj.x();
  ty = obj.y();

  clos.isOnMove    = false;
  clos.isReposMove = false;
  }

void MoveBehavior::repositionEvent(RepositionEvent &m) {
  clos.isReposMove = 0;

  if( !clos.isOnMove && !isMWalk ){
    tx = m.x;
    ty = m.y;

    clos.isReposMove = true;
    clos.isOnMove    = true;
    } else
    m.ignore();

  }

void MoveBehavior::positionChangeEvent( PositionChangeEvent & ) {
  clos.isReposMove = 0;

  tx = obj.x();
  ty = obj.y();
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
  }

void MoveBehavior::mouseDown(MyWidget::MouseEvent &e) {
  e.accept();
  }

void MoveBehavior::mouseUp( MyWidget::MouseEvent &e ) {
  if( e.button==MyWidget::MouseEvent::ButtonLeft ){
    if( obj.world().mouseObj()==0 ){
      obj.game().message( obj.playerNum(),
                          Move,
                          obj.world().mouseX(),
                          obj.world().mouseY()
                          );
      } else {
      GameObject *tg = obj.world().mouseObj();
      obj.game().message( obj.playerNum(),
                          ToUnit,
                          obj.world().objectWPtr(tg).id() );
      }
    }

  onRemoveHook();
  obj.game().removeHook( &hook );
  }

void MoveBehavior::onRemoveHook() {
  instaled = 0;
  }

void MoveBehavior::tick(const Terrain &terrain) {
  if( taget && timer==0 ){
    tx = taget.value().x();
    ty = taget.value().y();

    int d = obj.getClass().data.size +
            taget.value().getClass().data.size;
    d *= Terrain::quadSize;

    if( obj.distanceSQ(tx, ty) > d*d )
      calcWayAndMove( tx, ty,  terrain );

    timer = 15;
    }

  if( timer>0 )
    --timer;

  int qs = Terrain::quadSize;
  int x = obj.x()/qs,
      y = obj.y()/qs;

  int sz = 1;//obj.getClass().data.size;

  int bm = 1;

  if( !(clos.colisionDisp[0]==0 && clos.colisionDisp[1]==0) ){
    if( !clos.isOnMove && !clos.isReposMove ){
      bm = 2;

      int l = Math::distance( clos.colisionDisp[0], clos.colisionDisp[1],
                              0, 0 );
      l = std::max(l, 1);
      int s = obj.getClass().data.size*Terrain::quadSize/2;

      tx = obj.x() + clos.colisionDisp[0]*s/l;
      ty = obj.y() + clos.colisionDisp[1]*s/l;
      clos.colisionDisp[0] = 0;
      clos.colisionDisp[1] = 0;
      clos.isOnMove = true;
      clos.isReposMove = true;

      if( curentSpeed < obj.getClass().data.speed*Terrain::quadSize )
        curentSpeed = obj.getClass().data.speed*Terrain::quadSize;
      }
    }

  if( clos.isOnMove || clos.isReposMove ){
    step(terrain, sz);
    } else {
    bool lk = clos.isMVLock && x==clos.lkX && y==clos.lkY;
    if( !terrain.isEnableQuad( x, y, sz ) && !lk ){
      calcWayAndMove( obj.x(), obj.y(), terrain );
      }
    }

  }

void MoveBehavior::step(const Terrain &terrain, int sz ) {
  int acseleration = 8;

  int tx = this->tx,
      ty = this->ty;

  if( !clos.isReposMove ){
    tx = this->tx + clos.colisionDisp[0];
    ty = this->ty + clos.colisionDisp[1];

    if( !terrain.isEnableQuad( tx/Terrain::quadSize,
                               ty/Terrain::quadSize, 1 ) ){
      tx = this->tx;
      ty = this->ty;
      }
    }

  if( curentSpeed+acseleration < obj.getClass().data.speed )
    curentSpeed+=acseleration; else
    curentSpeed = obj.getClass().data.speed;

  int realL = Math::distance( obj.x(), obj.y(),
                              tx, ty );
  int l = realL/std::max(curentSpeed,1);

  if( ( ( 2*realL > obj.getClass().data.size*Terrain::quadSize ||
          way.size()>0 ) ||
        clos.isReposMove) &&
      l > 0 ){
    int x = obj.x()+(tx-obj.x())*curentSpeed/realL,
        y = obj.y()+(ty-obj.y())*curentSpeed/realL;

    if( l==1 ){
      x = tx;
      y = ty;
      }

    float wx = x/Terrain::quadSizef,
          wy = y/Terrain::quadSizef;

    int iwx = x/Terrain::quadSize,
        iwy = y/Terrain::quadSize;

    int pwx = obj.x()/Terrain::quadSize,
        pwy = obj.y()/Terrain::quadSize;

    bool ienable = terrain.isEnable( iwx, iwy );
    bool penable = terrain.isEnable( pwx, pwy );

    if( ienable || !penable ){
      int ltx = this->tx, lty = this->ty;

      obj.setPositionSmooth( x, y, terrain.heightAt(wx,wy) );
      obj.setViewDirection( ltx-obj.x(), lty-obj.y() );

      this->tx = ltx;
      this->ty = lty;
      } else {
      if( way.size() ){
        this->tx = way[0].x;
        this->ty = way[0].y;
        }

      if( !clos.isReposMove/* && !isMWalk*/ ){
        calcWayAndMove( this->tx, this->ty, terrain );
        }
      }

    } else {
    if( !nextPoint() ){
      clos.isOnMove     = false;
      clos.isReposMove  = false;
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

  int qs = Terrain::quadSize, hqs = qs/2;

  int mx = tx/qs,
      my = ty/qs;

  size_t id = v.size()-1;
  while( id<v.size() && id>0 && v[id].x==obj.x()/qs && v[id].y==obj.y()/qs ){
    --id;
    }

  bool isContinue = ( way.size() &&
                      id <  v.size() &&
                      mx == v[id].x &&
                      my == v[id].y );
  way  = v;

  if( v.size()==0 ){
    tx = obj.x();
    ty = obj.y();

    clos.isReposMove = false;
    clos.isOnMove    = false;
    return;
    }

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

  if( isContinue ){
    while( way.size() && way.back().x==obj.x()/qs && way.back().y==obj.y()/qs )
      way.pop_back();

    while( way.size() && way.back().x==mx && way.back().y==my )
      way.pop_back();
    } else {
    nextPoint();
    }
  }

void MoveBehavior::takeWay(const MoveBehavior &v) {
  if( this==&v )
    return;

  setWay(v.way);
  }

bool MoveBehavior::isSameDirection( const MoveBehavior &other ) {
  int dx1 = obj.x() - tx,
      dy1 = obj.y() - ty;

  int dx2 = other.obj.x() - other.tx,
      dy2 = other.obj.y() - other.ty;

  //int l1 = Math::sqrt( dx1*dx1 + dy1*dy1 );
  //int l2 = Math::sqrt( dx2*dx2 + dy2*dy2 );

  return ( dx1*dx2 + dy1*dy2 ) > 0;//l1*l2/8;
  }

void MoveBehavior::setupMoveHook() {
  if( !instaled ){
    instaled = obj.game().instalHook( &hook );
    }
  }

bool MoveBehavior::isCloseEnough( int x1, int y1,
                                  int x2, int y2,
                                  int unitSize ) {
  //int realL = Math::distance( x1, y1, x2, y2 );
  x1 -= x2;
  y1 -= y2;

  int realL = x1*x1 + y1*y1;
  int lc = unitSize*Terrain::quadSize;

  return 2*2*realL <= lc*lc;
  }
