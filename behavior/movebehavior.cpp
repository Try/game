#include "movebehavior.h"

#include "game/gameobject.h"
#include "util/math.h"
#include "game/world.h"

#include "algo/algo.h"
#include "algo/wayfindalgo.h"

#include "game.h"

MoveBehavior::MoveBehavior( GameObject &object,
                            Behavior::Closure & c )
  :obj(object), clos(c), intentToHold(c.intentToHold), isMWalk(c.isMineralMove) {
  tx = 0;
  ty = 0;

  timer = 0;

  isWayAcept = 0;
  isMWalk    = 0;

  curentSpeed = 0;
  intentToHold = 0;

  intentPos[0] = 0;
  intentPos[1] = 0;

  clos.colisionDisp[0] = 0;
  clos.colisionDisp[1] = 0;

  clos.isMoviable = true;
  inputMode = inMove;

  way.reserve(256);

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
  unHold();

  clos.isReposMove = 0;
  clos.isOnMove    = true;

  way.clear();
  isWayAcept = 1;
  isMWalk    = 0;

  tx = m.x;
  ty = m.y;

  obj.world().game.message( obj.playerNum(),
                            AtackMoveGroup, m.x, m.y, m.modif );
  }

void MoveBehavior::atackContinueEvent(MoveSingleEvent &m) {
  int qs = Terrain::quadSize/2;

  if( way.size() &&
      m.x/qs==way[0].x/qs &&
      m.y/qs==way[0].y/qs &&
      clos.isOnMove )
    return;

  moveEvent(m);
  }

void MoveBehavior::moveEvent( MoveSingleEvent &m ) {
  clos.isPatrul = false;

  taget = WeakWorldPtr();

  clos.isOnMove    = true;
  clos.isReposMove = 0;
  isMWalk    = 0;

  calcWayAndMove( m.x, m.y, obj.world().terrain() );
  }

void MoveBehavior::moveEvent( MoveEvent &m ) {
  clos.isPatrul = false;

  clos.isReposMove = 0;
  taget = WeakWorldPtr();
  unHold();

  isWayAcept = 1;
  isMWalk    = 0;
  obj.world().game.message( obj.playerNum(), MoveGroup, m.x, m.y, m.modif );
  }

void MoveBehavior::moveEvent(MoveToUnitEvent &m) {
  clos.isPatrul = false;

  clos.isReposMove = 0;
  unHold();

  taget = obj.world().objectWPtr( m.id );
  timer = 0;

  isWayAcept = 1;
  isMWalk    = 0;
  obj.world().game.message( obj.playerNum(),
                            MoveToUnitGroup,
                            m.id,
                            m.modif );
  }

void MoveBehavior::moveEvent( MineralMoveEvent &m ) {
  clos.isPatrul = false;

  taget = WeakWorldPtr();
  unHold();

  clos.isReposMove = 0;

  way.clear();
  isMWalk = 1;

  tx = m.x;
  ty = m.y;

  clos.isOnMove    = true;
  clos.isReposMove = false;
  }

void MoveBehavior::moveEvent( PatrulEvent &m ) {
  Point p1 = { obj.x(), obj.y() },
        p2 = { m.x,     m.y     };

  patrulWay.clear();
  patrulWay.push_back(p1);
  patrulWay.push_back(p2);
  patrulPoint = 0;

  clos.isPatrul = true;
  }

void MoveBehavior::stopEvent(StopEvent &) {
  //patrulWay.clear();

  clos.isReposMove = 0;
  clos.isPatrul = false;
  unHold();

  way.clear();
  taget = WeakWorldPtr();

  isMWalk = 0;

  tx = obj.x();
  ty = obj.y();

  clos.isOnMove    = false;
  clos.isReposMove = false;
  }

void MoveBehavior::holdEvent( HoldEvent & ) {
  clos.isPatrul = false;

  clos.isReposMove = 0;
  unHold();

  way.clear();
  taget = WeakWorldPtr();

  isMWalk = 0;

  tx = obj.x();
  ty = obj.y();

  clos.isOnMove    = false;
  clos.isReposMove = false;

  intentToHold = 1;
  }

void MoveBehavior::cancelEvent(CancelEvent &) {
  clos.isPatrul = false;

  clos.isReposMove = 0;
  unHold();

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
  }

void MoveBehavior::calcWayAndMove( int tx, int ty,
                                   const Terrain & /*terrain*/ ) {
  isWayAcept = 1;
  obj.world().wayFind( tx, ty, &obj );
  }

void MoveBehavior::mouseDown(Tempest::MouseEvent &e) {
  e.accept();
  }

void MoveBehavior::mouseUp( Tempest::MouseEvent &e ) {
  if( e.button==Tempest::MouseEvent::ButtonLeft ){
    patrulWay.clear();
    if( inputMode==inMove ){
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
      } else {
      obj.game().message( obj.playerNum(),
                          Patrul,
                          obj.world().mouseX(),
                          obj.world().mouseY()
                          );
      obj.world().emitHudAnim( "hud/blink",
                               obj.x(),
                               obj.y(),
                               0.1);
      obj.world().emitHudAnim( "hud/blink",
                               obj.world().mouseX(),
                               obj.world().mouseY(),
                               0.1);
      }
    }

  //onRemoveHook();
  obj.game().removeHook( &hook );
  }

void MoveBehavior::onRemoveHook() {
  instaled = 0;
  }

void MoveBehavior::unHold() {
  if( intentToHold==2 && clos.isMVLock ){
    clos.isMVLock = false;
    obj.world().terrain().editBuildingsMap( clos.lkX, clos.lkY, 2, 2, -1 );
    }

  intentToHold  = 0;
  }

void MoveBehavior::tick(const Terrain &terrain) {
  intentPos[0] = obj.x();
  intentPos[1] = obj.y();

  if( timer>0 )
    --timer;

  if( taget && timer==0 ){
    tx = taget.value().x();
    ty = taget.value().y();

    int d = obj.getClass().data.size +
            taget.value().getClass().data.size;
    d *= Terrain::quadSize;

    calcWayAndMove( tx, ty,  terrain );
    timer = 15;
    return;
    }

  if( !clos.isOnMove && clos.isPatrul && !clos.isMVLock ){
    patrulPoint = (patrulPoint+1)%patrulWay.size();

    Point &p = patrulWay[patrulPoint];
    //calcWayAndMove( p.x, p.y, terrain );
    obj.behavior.message( AtackMoveContinue, p.x, p.y );
    clos.isPatrul = true;
    }

  int qs = Terrain::quadSize;
  int x = obj.x()/qs,
      y = obj.y()/qs;

  if( !(clos.colisionDisp[0]==0 && clos.colisionDisp[1]==0) ){
    if( !clos.isOnMove && !clos.isReposMove ){
      int l = Math::distance( clos.colisionDisp[0], clos.colisionDisp[1],
                              0, 0 );
      l = 2*std::max(l, 1);
      int s = obj.getClass().data.size*Terrain::quadSize;
      //int s = obj.getClass().data.speed*Terrain::quadSize;

      tx = obj.x() + clos.colisionDisp[0]*s/l;
      ty = obj.y() + clos.colisionDisp[1]*s/l;
      clos.colisionDisp[0] = 0;
      clos.colisionDisp[1] = 0;
      clos.isOnMove = true;
      clos.isReposMove = true;

      //clos.;

      if( curentSpeed < obj.getClass().data.speed*Terrain::quadSize )
        curentSpeed = obj.getClass().data.speed*Terrain::quadSize;
      }
    }

  if( clos.isOnMove || clos.isReposMove ){
    step(terrain);
    } else {
    bool lk = clos.isMVLock && x==clos.lkX && y==clos.lkY;
    if( !terrain.isEnableQuad( x, y, 1 ) && !lk ){
      calcWayAndMove( obj.x(), obj.y(), terrain );
      return;
      }
    }

  }

void MoveBehavior::step(const Terrain &terrain ) {
  int acseleration = obj.getClass().data.acseleration;

  int tx = this->tx,
      ty = this->ty;

  if( !clos.isReposMove ){
    tx = this->tx;// + clos.colisionDisp[0];
    ty = this->ty;// + clos.colisionDisp[1];

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
    int dx = tx-obj.x(),
        dy = ty-obj.y();

    int x = obj.x()+dx*curentSpeed/realL,
        y = obj.y()+dy*curentSpeed/realL;

    if( l==1 ){
      x = tx;
      y = ty;
      }

    //float wx = x/Terrain::quadSizef,
    //      wy = y/Terrain::quadSizef;

    int iwx = x/Terrain::quadSize,
        iwy = y/Terrain::quadSize;

    int pwx = obj.x()/Terrain::quadSize,
        pwy = obj.y()/Terrain::quadSize;

    bool ienable = terrain.isEnable( iwx, iwy );
    bool penable = terrain.isEnable( pwx, pwy );

    if( ienable || !penable ){
      int ltx = this->tx, lty = this->ty;

      intentPos[0] = x;
      intentPos[1] = y;

      if( !clos.isMVLock )
        obj.setViewDirection( ltx-obj.x(), lty-obj.y() );

      this->tx = ltx;
      this->ty = lty;
      } else {
      if( way.size() ){
        this->tx = way[0].x;
        this->ty = way[0].y;
        }

      if( !clos.isReposMove/* && !isMWalk*/ ){
        const int qs = Terrain::quadSize;
        intentPos[0] = (obj.x()/qs)*qs + qs/2;
        intentPos[1] = (obj.y()/qs)*qs + qs/2;

        setPositon(intentPos[0], intentPos[1]);
        calcWayAndMove( this->tx, this->ty, terrain );
        return;
        }

      clos.isReposMove = false;
      }

    } else {
    //int qs = Terrain::quadSize;
    //intentPos[0] = (obj.x()/qs)*qs + qs/2;
    //intentPos[1] = (obj.y()/qs)*qs + qs/2;

    intentPos[0] = tx;
    intentPos[1] = ty;

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

  tx = obj.x();
  ty = obj.y();

  if( v.size()==0 ){
    clos.isReposMove = false;
    clos.isOnMove    = false;
    return;
    }

  int dx = v.back().x*qs + hqs - obj.x(),
      dy = v.back().y*qs + hqs - obj.y();

  /*
  if( !obj.world().terrain().isEnableQuad( obj.x()/qs, obj.y()/qs, 2) ){
    dx = 0;
    dy = 0;
    }*/

  int x0 = (obj.x()   )/qs,
      y0 = (obj.y()   )/qs,
      x1 = x0,
      y1 = y0;
  if( dx>0 )
    ++x1;
  if( dx<0 )
    --x1;

  if( dy>0 )
    ++y1;
  if( dy<0 )
    --y1;

  if( !( obj.world().terrain().isEnable( x0, y0 ) &&
         obj.world().terrain().isEnable( x1, y0 ) &&
         obj.world().terrain().isEnable( x0, y0 ) &&
         obj.world().terrain().isEnable( x1, y1 ) ) ){
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
    inputMode = inMove;
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

  return abs(x1)<=lc &&
         abs(y1)<=lc &&
         2*2*realL <= lc*lc;
  }

void MoveBehavior::updatePos( const Terrain &t ) {
  int ux = obj.x()/Terrain::quadSize,
      uy = obj.y()/Terrain::quadSize;
  bool uenable = t.isEnableQuad( ux, uy, 1 );

  if( intentToHold==1 ){
    if( clos.isMVLock==0 && uenable ){
      clos.isMVLock = true;
      clos.lkX = ux;
      clos.lkY = uy;
      obj.world().terrain().editBuildingsMap( clos.lkX, clos.lkY, 2, 2, 1 );
      intentToHold = 2;
      }
    }

  if( !clos.isOnMove )
    return;

  if( intentToHold==2 )
    return;

  int l = Math::distance( clos.colisionDisp[0], clos.colisionDisp[1],
                          0, 0 );
  l = 2*std::max(l, 1);
  int s = obj.getClass().data.speed;

  int x = intentPos[0],
      y = intentPos[1],
      tx = x + clos.colisionDisp[0]*s/l,
      ty = y + clos.colisionDisp[1]*s/l;

  if( !uenable || (
      t.isEnable( tx/Terrain::quadSize, ty/Terrain::quadSize ) &&
      abs(x/Terrain::quadSize - tx/Terrain::quadSize)+
      abs(y/Terrain::quadSize - ty/Terrain::quadSize)<=1) ){
    x = tx;
    y = ty;
    }

  float wx = x/Terrain::quadSizef,
        wy = y/Terrain::quadSizef;

  obj.setPositionSmooth( x, y, t.heightAt(wx,wy) );
  }

void MoveBehavior::setPositon(int x, int y) {
  float wx = x/Terrain::quadSizef,
        wy = y/Terrain::quadSizef;

  if( obj.world().terrain().isEnableQuad( x/Terrain::quadSize, y/Terrain::quadSize, 1 ) )
    obj.setPositionSmooth( x, y, obj.world().terrain().heightAt(wx,wy) );
  }

void MoveBehavior::setupPatrul() {
  if( !instaled ){
    instaled  = obj.game().instalHook( &hook );
    inputMode = inPatrul;
    }
  }
