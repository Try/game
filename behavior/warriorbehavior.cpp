#include "warriorbehavior.h"
#include "game/gameobject.h"
#include "game.h"

#include "game/bullet.h"
#include "movebehavior.h"

WarriorBehavior::WarriorBehavior( GameObject &obj,
                                  Behavior::Closure &c )
  :obj(obj), mvLock(c.isMVLock), lkX(c.lkX), lkY(c.lkY) {
  isAtk    = false;
  dAtkTime = 0;
  mvLock   = 0;

  lastX = obj.x();
  lastY = obj.y();

  isAClick = 0;
  acX = 0;
  acY = 0;

  instaled = 0;
  hook.mouseDown.bind( *this, &WarriorBehavior::mouseDown    );
  hook.mouseUp  .bind( *this, &WarriorBehavior::mouseUp      );
  hook.onRemove .bind( *this, &WarriorBehavior::onRemoveHook );
  }

WarriorBehavior::~WarriorBehavior() {
  if( mvLock ){
    obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
    }
  }

void WarriorBehavior::tick( const Terrain & ) {
  if( obj.getClass().data.atk.size() == 0 )
    return;

  if( dAtkTime>0 )
    --dAtkTime;

  if( dAtkTime )
    return;

  if( obj.isOnMove() && !( isAtk || isAClick ) )
    return;

  if( !obj.isOnMove() && isAClick && !taget ){
    obj.behavior.message( AtackMoveContinue, acX, acY );
    //isAClick = obj.isOnMove();
    }

  if( isAClick &&
      !mvTaget &&
      MoveBehavior::isCloseEnough( obj.x(), obj.y(),
                                   acX, acY, obj.getClass().data.size ) ){
    isAClick = false;
    }

  dAtkTime = 5;

  GameObject * rawTg      = 0;
  GameObject * rawTgBuild = 0;
  int dTg = 0, dBuild = 0;

  GameObject * tg = 0;
  if( mvTaget ){
    acX = mvTaget.value().x();
    acY = mvTaget.value().y();
    } else {
    obj.world().spatial().visit( obj.x(),
                                 obj.y(),
                                 obj.getClass().data.visionRange,
                                 &lookOn,
                                 obj,
                                 this,
                                 rawTg,
                                 rawTgBuild,
                                 dTg,
                                 dBuild );

    tg = rawTgBuild;
    takeTaget( tg, rawTgBuild, dBuild );
    takeTaget( tg, rawTg,      dTg    );
    }

  taget = mvTaget;

  if( !obj.isOnMove() || isAtk || (obj.distanceQL(lastX, lastY)>1) ){
    if( mvTaget )
      taget = mvTaget; else
    if( tg )
      taget = obj.world().objectWPtr( tg );

    tickAtack( mvTaget==taget );
    } else {
    if( mvLock ){
      obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
      mvLock = 0;
      }
    }
  }

void WarriorBehavior::lookOn( GameObject &tg,
                              GameObject &obj,
                              WarriorBehavior * /*wb*/,
                              GameObject *& rawTg,
                              GameObject *& rawTgBuild,
                              int & dTg,
                              int & dBuld ) {
  if( tg.getClass().data.invincible )
    return;

  if( tg.playerNum()==0 )
    return;

  if( tg.team()==obj.team() )
    return;

  int d  = tg.distanceSQ( obj.x(), obj.y() );

  if( tg.behavior.find<WarriorBehavior>() ){
    if( rawTg==0 || d<dTg ){
      dTg = d;
      rawTg = &tg;
      }
    } else {
    if( rawTgBuild==0 || d<dBuld ){
      dBuld = d;
      rawTgBuild = &tg;
      }
    }
  }

void WarriorBehavior::takeTaget( GameObject *&out, GameObject *tg, int /*d*/ ){
  if( tg!=0 ){
    int vrange = (obj.getClass().data.visionRange +
                  tg->getClass().data.size )*Terrain::quadSize;
    vrange = vrange*vrange;

    if( tg->distanceSQ(obj.x(), obj.y()) < vrange ){
      out = tg;
      }
    }
  }

bool WarriorBehavior::message( AbstractBehavior::Message msg,
                               int x, int y,
                               AbstractBehavior::Modifers md) {
  if( msg==Move ||
      msg==MoveGroup ||
      msg==MineralMove ||
      msg==MoveSingle  ||
      msg==Cancel ){
    mvTaget = WeakWorldPtr();

    isAtk    = false;
    isAClick = false;

    if( mvLock ){
      obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
      mvLock = 0;
      }
    }

  if( msg==AtackMove ||
      msg==AtackMoveGroup ){
    mvTaget = WeakWorldPtr();

    isAClick = true;
    acX = x;
    acY = y;
    }

  return AbstractBehavior::message(msg, x, y, md);
  }

bool WarriorBehavior::message( Message msg, size_t id,
                               AbstractBehavior::Modifers md) {
  if( msg==ToUnit || msg==AtackToUnit ){
    WeakWorldPtr ptr = obj.world().objectWPtr(id);
    if( (ptr.value().team()!=obj.team() || msg==AtackToUnit) &&
        !ptr.value().getClass().data.invincible ){
      mvTaget = ptr;
      isAClick = true;
      acX = mvTaget.value().x();
      acY = mvTaget.value().y();
      return 1;
      }
    } else {
    mvTaget = WeakWorldPtr();

    isAtk    = false;
    isAClick = false;

    if( mvLock ){
      obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
      mvLock = 0;
      }
    }

  return AbstractBehavior::message(msg, id, md);
  }

void WarriorBehavior::aClick() {
  if( !instaled ){
    instaled = obj.game().instalHook( &hook );
    }
  }

void WarriorBehavior::move(int x, int y) {
  int qs = Terrain::quadSize;

  x /= qs;
  y /= qs;

  bool isAClickP = isAClick;
  obj.behavior.message( AtackMoveContinue, x*qs + qs/2, y*qs + qs/2 );
  isAClick = isAClickP;

  isAtk = true;

  dAtkTime = 40;
  for( size_t i=0; i<obj.getClass().data.atk.size(); ++i ){
    if( obj.getClass().data.atk[i].range>0 )
      dAtkTime = 10;
    }

  lastX = obj.x();
  lastY = obj.y();
  }

void WarriorBehavior::damageTo(GameObject &dobj) {
  dAtkTime = obj.getClass().data.atk[0].delay;
  int absDmg = obj.getClass().data.atk[0].damage;

  obj.setViewDirection( dobj.x() - obj.x(),
                        dobj.y() - obj.y() );

  if( obj.getClass().data.atk[0].range>0 &&
      obj.getClass().data.atk[0].bullet.size() ){
    auto bul = dobj.reciveBulldet( obj.getClass().data.atk[0].bullet );
    Bullet& b = *bul;

    b.x = obj.x();
    b.y = obj.y();
    b.view.teamColor = obj.teamColor();

    b.z   = obj.viewHeight()/2;
    b.tgZ = dobj.viewHeight()/2;

    b.absDmg = absDmg;
    b.tick();
    obj.world().game.resources().sound("fire_ball").play();
    } else {
    obj.world().game.resources().sound("attack-sword-001").play();
    dobj.setHP( dobj.hp() - absDmg );
    }

  obj.behavior.message( StopMove, 0,0 );
  }

void WarriorBehavior::positionChangeEvent(PositionChangeEvent &) {
  if( mvLock ){
    obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
    mvLock = 0;
    }
  }

void WarriorBehavior::tickAtack( bool ignoreVrange ) {
  if( taget ){
    int vrange = obj.getClass().data.visionRange*Terrain::quadSize;
    int arange = (  obj.getClass().data.atk[0].range +
                   (obj.getClass().data.size + 2 +
                    taget.value().getClass().data.size)/2 )*Terrain::quadSize;

    vrange = vrange*vrange;
    arange = arange*arange;
    int d = taget.value().distanceSQ(obj.x(), obj.y());

    if( d <= arange ){
      damageTo( taget.value() );
      if( !mvLock ){
        lkX = obj.x()/Terrain::quadSize;
        lkY = obj.y()/Terrain::quadSize;

        obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, +1 );
        mvLock = 1;
        }
      } else
    if( d <= vrange || ignoreVrange )
      move( taget.value().x(), taget.value().y() );
    } else {
    if( mvLock ){
      obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
      mvLock = 0;
      }
    }
  }

void WarriorBehavior::mouseDown(MyWidget::MouseEvent &e) {
  e.accept();
  }

void WarriorBehavior::mouseUp( MyWidget::MouseEvent &e ) {
  if( e.button==MyWidget::MouseEvent::ButtonLeft ){
    if( obj.world().mouseObj()==0 ){
      obj.game().message( obj.playerNum(),
                          AtackMove,
                          obj.world().mouseX(),
                          obj.world().mouseY()
                          );
      } else {
      GameObject *tg = obj.world().mouseObj();
      obj.game().message( obj.playerNum(),
                          AtackToUnit,
                          obj.world().objectWPtr(tg).id() );
      }
    }

  onRemoveHook();
  obj.game().removeHook( &hook );
  }

void WarriorBehavior::onRemoveHook() {
  instaled = 0;
  }
