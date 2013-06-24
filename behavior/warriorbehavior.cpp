#include "warriorbehavior.h"
#include "game/gameobject.h"
#include "game.h"

#include "game/bullet.h"
#include "movebehavior.h"

#include <cmath>
#include "util/math.h"

WarriorBehavior::WarriorBehavior( GameObject &obj,
                                  Behavior::Closure &c )
  : obj(obj), intentToHold(c.intentToHold),
    isPatrul(c.isPatrul),
    mvLock(c.isMVLock), lkX(c.lkX), lkY(c.lkY) {
  isAtk    = false;
  dAtkTime = 0;
  dRqTime  = 0;

  mvLock   = 0;
  hasAtkTaget = false;

  lastX = obj.x();
  lastY = obj.y();

  isAClick = 0;
  acX = 0;
  acY = 0;

  lkX = 0;
  lkY = 0;

  instaled = 0;
  hook.mouseDown.bind( *this, &WarriorBehavior::mouseDown    );
  hook.mouseUp  .bind( *this, &WarriorBehavior::mouseUp      );
  hook.onRemove .bind( *this, &WarriorBehavior::onRemoveHook );
  }

WarriorBehavior::~WarriorBehavior() {  
  unlockGround();
  }

void WarriorBehavior::tick( const Terrain & ) {
  if( obj.getClass().data.atk.size() == 0 )
    return;

  if( !isAClick ){
    int dummy;
    (void)dummy;
    }

  if( dAtkTime>0 )
    --dAtkTime;

  if( dRqTime>0 )
    --dRqTime;

  if( hasAtkTaget && !taget ){
    dRqTime = 0;
    hasAtkTaget = false;
    }

  if( isAClick && !obj.isOnMove() && !hasAtkTaget )
    dRqTime = 0;

  if( taget && obj.isOnMove() && dAtkTime>0 ){
    if( canShoot(taget.value()) ){
      tickAtack( true );
      return;
      }
    }

  if( dAtkTime && dRqTime )
    return;

  if( obj.isOnMove() && !( isAtk || isAClick ) )
    return;

  if( isAClick &&
      !mvTaget &&
      MoveBehavior::isCloseEnough( obj.x(), obj.y(),
                                   acX, acY, obj.getClass().data.size ) ){
    isAClick = false;
    }

  dRqTime = std::max( dRqTime, 10 );

  GameObject * rawTg      = 0;
  GameObject * rawTgBuild = 0;
  int dTg = 0, dBuild = 0;

  GameObject * tg = 0;
  if( mvTaget ){
    acX = mvTaget.value().x();
    acY = mvTaget.value().y();
    } else {
    if( !taget || !canShoot( taget.value() ) ){
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
      } else {
      tg = &taget.value();
      }
    }

  if( !obj.isOnMove() && mvTaget )
    taget = mvTaget;

  if( !obj.isOnMove() || ( isAtk || isAClick )  ){
    if( mvTaget )
      taget = mvTaget; else
    if( tg && tg!=&taget.value() )
      taget = obj.world().objectWPtr( tg );

    tickAtack( mvTaget==taget );
    } else {    
    unlockGround();
    }

  if( !obj.isOnMove() && isAClick && !taget ){
    if( intentToHold==0 )
      obj.behavior.message( AtackMoveContinue, acX, acY );
    }

  hasAtkTaget = bool(taget);
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
  if( msg==MoveSingle ){
    mvTaget = WeakWorldPtr();

    isAtk    = false;
    isAClick = false;

    unlockGround();
    }
  if( msg==Move ||
      msg==MoveGroup ||
      msg==MineralMove ||
      msg==Cancel ){
    mvTaget = WeakWorldPtr();

    isAtk    = false;
    isAClick = false;

    unlockGround();
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

    unlockGround();
    }

  return AbstractBehavior::message(msg, id, md);
  }

void WarriorBehavior::aClick() {
  if( !instaled ){
    instaled = obj.game().instalHook( &hook );
    }
  }

void WarriorBehavior::move( int x, int y ) {
  if( intentToHold!=0 )
    return;

  int qs = Terrain::quadSize;

  x /= qs;
  y /= qs;

  bool isAClickP = isAClick;
  bool isPat     = isPatrul;
  obj.behavior.message( AtackMoveContinue, x*qs + qs/2, y*qs + qs/2 );
  isAClick = isAClickP;
  isPatrul = isPat;

  isAtk = true;

  dRqTime = std::min(50, 30+(obj.distanceQ(x,y)*4));
  /*
  for( size_t i=0; i<obj.getClass().data.atk.size(); ++i ){
    if( obj.getClass().data.atk[i].range>0 ){
      dRqTime = 40;
      }
    }*/

  lastX = obj.x();
  lastY = obj.y();
  }

void WarriorBehavior::damageTo(GameObject &dobj) {
  const ProtoObject::GameSpecific::Atack *ax = 0;

  const std::vector<ProtoObject::GameSpecific::Atack>& atk = obj.getClass().data.atk;
  const ProtoObject& cls = dobj.getClass();

  for( size_t i=0; i<atk.size(); ++i ){
    bool ok = false;
    if( atk[i].uDestType!=size_t(-1) ){
      for( size_t r=0; r<cls.data.utype.size(); ++r )
        if( cls.data.utype[r]==atk[i].uDestType )
          ok = true;
      } else {
      ok = true;
      }

    if( ok ){
      if( ax==0 || ax->damage*atk[i].delay < atk[i].damage*ax->delay )
        ax = &atk[i];
      }
    }

  obj.setViewDirection( dobj.x() - obj.x(),
                        dobj.y() - obj.y() );

  const ProtoObject::GameSpecific::Atack& a = *ax;
  dAtkTime = a.delay;

  if( a.range>0 && a.bullet.size() ){
    auto bul = dobj.reciveBulldet( a.bullet );
    Bullet& b = *bul;

    b.x = obj.x();
    b.y = obj.y();
    b.setTeamColor( obj.teamColor() );

    b.z   = obj.viewHeight()/2  + World::coordCast(obj.z() );
    b.tgZ = dobj.viewHeight()/2 + World::coordCast(dobj.z());

    b.plOwner      = obj.player().number();
    b.atack        = a;

    b.tick();
    obj.world().game.resources().sound("fire_ball").play();
    } else {
    //obj.world().game.resources().sound("attack-sword-001").play();
    mkDamage( dobj, obj.playerNum(),
              dobj.x(), dobj.y(),
              a );
    }

  if( intentToHold==0 ){
    bool isPat     = isPatrul;
    obj.behavior.message( StopMove, 0,0 );
    isPatrul = isPat;
    }
  }

void WarriorBehavior::mkDamage( GameObject &dobj,
                                int plOwner,
                                int x, int y,
                                const ProtoObject::GameSpecific::Atack &atk) {
  int absDmg = std::max( atk.damage - dobj.getClass().data.armor, 0 );
  dobj.setHP( dobj.hp() - absDmg );

  if( atk.splashSize ){
    dobj.world().spatial().visit( dobj.x(),
                                  dobj.y(),
                                  atk.splashSize,
                                  &damageSplash,
                                  plOwner,
                                  x, y,
                                  dobj,
                                  atk );
    }
  }

void WarriorBehavior::damageSplash( GameObject & tg ,
                                    int plOwner,
                                    int x, int y,
                                    GameObject & nobj,
                                    const ProtoObject::GameSpecific::Atack &a ){
  if( tg.getClass().data.invincible )
    return;

  if( tg.playerNum()==0 )
    return;

  if( tg.team()==tg.game().player(plOwner).team() )
    return;

  if( &tg==&nobj )
    return;

  int ss = a.splashSize*Terrain::quadSize;

  int d  = tg.distanceSQ( x, y );
  if( d < ss*ss ){
    int absDmg = std::max(a.splashDamage - tg.getClass().data.armor, 0);
    tg.setHP( tg.hp()-absDmg );

    float mv[2] = { float(x - tg.x()), float(y - tg.y()) };
    float l = -sqrt(mv[0]*mv[0] + mv[1]*mv[1])/11;
    mv[0]/=l;
    mv[1]/=l;

    tg.incDieVec( mv[0], mv[1], 11 );
    }
  }

void WarriorBehavior::positionChangeEvent(PositionChangeEvent &) {
  dRqTime = 0;
  unlockGround();
  }

bool WarriorBehavior::canShoot( GameObject &taget ) {
  for( size_t i=0; i<obj.getClass().data.atk.size(); ++i )
    if( canShoot(taget, obj.getClass().data.atk[i]) )
      return true;

  return false;
  }

bool WarriorBehavior::canShoot( GameObject &taget,
                                const ProtoObject::GameSpecific::Atack & atk ) {
  if( atk.uDestType!=size_t(-1) ){
    bool ok = false;
    for( size_t i=0; i<taget.getClass().data.utype.size(); ++i )
      if( taget.getClass().data.utype[i]==atk.uDestType ){
        ok = true;
        break;
        }

    if( !ok )
      return false;
    }

  int vrange = obj.getClass().data.visionRange*Terrain::quadSize;
  int arange = (  atk.range +
                 (obj.getClass().data.size + 2 +
                  taget.getClass().data.size)/2 )*Terrain::quadSize;

  vrange = vrange*vrange;
  arange = arange*arange;
  int d = taget.distanceSQ(obj.x(), obj.y());

  return ( d <= arange );
  }

void WarriorBehavior::tickAtack( bool ignoreVrange ) {
  int d = 0;

  if( taget ){
    if( canShoot( taget.value() ) ){
      bool isPat     = isPatrul;
      obj.behavior.message( StopMove, 0,0 );
      isPatrul = isPat;

      if( dAtkTime==0 ){
        damageTo( taget.value() );
        lockGround();
        }
      } else{
      int vrange = obj.getClass().data.visionRange*Terrain::quadSize;
      int arange = 0;

      const std::vector<size_t>& id = taget.value().getClass().data.utype;
      for( size_t i=0; i<obj.getClass().data.atk.size(); ++i )
        for( size_t r=0; r<id.size(); ++r ){
          if( obj.getClass().data.atk[i].uDestType==size_t(-1) ||
              obj.getClass().data.atk[i].uDestType==id[r] ){
            arange = (  obj.getClass().data.atk[i].range +
                       (obj.getClass().data.size + 2 +
                        taget.value().getClass().data.size)/2 )*Terrain::quadSize;
            }
          }

      vrange = vrange*vrange;

      //int oldAR = arange;
      //arange = arange*arange;
      d = taget.value().distanceSQ(obj.x(), obj.y());

      if( d <= vrange || ignoreVrange ){
        if( obj.distanceQL(lastX, lastY)>0 || !obj.isOnMove() ){
          unlockGround();

          if( taget.value().getClass().data.speed>0 ){
            int dx = taget.value().x()-obj.x(),
                dy = taget.value().y()-obj.y();

            int x = obj.x(),
                y = obj.y();

            if( abs(dx)>2*Terrain::quadSize && abs(dy)>2*Terrain::quadSize ){
              x += dx/2;
              y += dy/2;
              } else {
              x += dx;
              y += dy;
              }

            move( x, y );
            } else {
            int x = taget.value().x(),
                y = taget.value().y(),
                dx = (x-obj.x()),
                dy = (y-obj.y()),
                l  = Math::sqrt(dx*dx+dy*dy);

            if( arange>2*Terrain::quadSize )
              arange -= 2*Terrain::quadSize;

            if( l>arange && l>0 ){
              dx = dx*arange/l;
              dy = dy*arange/l;
              } else {
              dx = 0;
              dy = 0;
              }

            if( obj.world().terrain().isEnable(x-dx, y-dy) )
              move( x-dx, y-dy ); else
              move( x, y );
            }
          }
        } else {
        //std::cout << d << std::endl;
        taget = WeakWorldPtr();
        }
      }
    } else {
    unlockGround();
    }
  }

void WarriorBehavior::mouseDown(Tempest::MouseEvent &e) {
  e.accept();
  }

void WarriorBehavior::mouseUp( Tempest::MouseEvent &e ) {
  if( e.button==Tempest::MouseEvent::ButtonLeft ){
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

void WarriorBehavior::lockGround() {
  if( !mvLock ){
    lkX = obj.x()/Terrain::quadSize;
    lkY = obj.y()/Terrain::quadSize;

    //obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, +1 );
    mvLock = 1;
    }
  }

void WarriorBehavior::unlockGround() {
  if( mvLock && intentToHold!=2 ){
    //obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
    mvLock = 0;
    }
  }
