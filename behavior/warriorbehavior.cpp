#include "warriorbehavior.h"
#include "game/gameobject.h"
#include "game.h"

#include "game/bullet.h"

WarriorBehavior::WarriorBehavior( GameObject &obj,
                                  Behavior::Closure &c ):obj(obj) {
  isAtk    = false;
  dAtkTime = 0;
  mvLock   = 0;

  lastX = obj.x();
  lastY = obj.y();
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

  size_t id = -1, idA   = -1;
  int dist  = -1, distA = -1;

  for( size_t i=0; i<obj.world().objectsCount(); ++i ){
    GameObject & tg = obj.world().object(i);
    int d = tg.distanceSQ( obj.x(), obj.y() );

    int dd = dist;
    bool w = ( tg.behavior.find<WarriorBehavior>() );
    if( w )
      dd = distA;

    if( tg.team()!=obj.team() &&
        tg.team()!=0 &&
        (dd < 0 || d<dd) ){
      if( w ){
        idA   = i;
        distA = d;
        } else{
        id   = i;
        dist = d;
        }
      }
    }

  if( idA!=size_t(-1) ){
    int vrange = obj.getClass().data.visionRange*Terrain::quadSize;
    vrange = vrange*vrange;

    if( obj.world().objectWPtr(idA).value().distanceSQ(obj.x(), obj.y())
        < vrange ){
      id = idA;
      dist = distA;
      }
    }

  if( !obj.isOnMove() || isAtk || (obj.distanceQL(lastX, lastY)>1) ){
    if( id!=size_t(-1) )
      taget = obj.world().objectWPtr(id);

    if( taget ){
      int vrange = obj.getClass().data.visionRange*Terrain::quadSize;
      int arange = (  obj.getClass().data.atk[0].range +
                     (obj.getClass().data.size + 1 +
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
      if( d <= vrange )
        move( taget.value().x(), taget.value().y() );
      } else {
      if( mvLock ){
        obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
        mvLock = 0;
        }
      }
    } else {
    if( mvLock ){
      obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
      mvLock = 0;
      }
    }
  }

bool WarriorBehavior::message( AbstractBehavior::Message msg,
                               int /*x*/, int /*y*/,
                               AbstractBehavior::Modifers /*md*/) {
  if( msg==Move ||
      msg==MoveGroup ||
      msg==MineralMove ||
      msg==MoveSingle ){
    isAtk = false;

    if( mvLock ){
      obj.world().terrain().editBuildingsMap( lkX, lkY, 1, 1, -1 );
      mvLock = 0;
      }
    }

  return false;
  }

void WarriorBehavior::move(int x, int y) {
  int qs = Terrain::quadSize;

  x /= qs;
  y /= qs;
  obj.behavior.message( MoveSingle, x*qs + qs/2, y*qs + qs/2 );

  isAtk = true;

  dAtkTime = 40;
  lastX = obj.x();
  lastY = obj.y();
  }

void WarriorBehavior::damageTo(GameObject &dobj) {
  dAtkTime = obj.getClass().data.atk[0].delay;
  int absDmg = obj.getClass().data.atk[0].damage;

  obj.setViewDirection( dobj.x() - obj.x(),
                        dobj.y() - obj.y() );

  if( obj.getClass().data.atk[0].range>0 ){
    auto bul = dobj.reciveBulldet("worker");
    Bullet& b = *bul;

    b.x = obj.x();
    b.y = obj.y();
    b.view.teamColor = obj.teamColor();
    b.view.setSelectionVisible(0);

    b.z   = obj.viewHeight()/2;
    b.tgZ = dobj.viewHeight()/2;

    b.absDmg = absDmg;
    b.tick();
    } else {
    dobj.setHP( dobj.hp() - absDmg );
    }

  obj.behavior.message( StopMove, 0,0 );
  }
