#include "bonusbehavior.h"

#include "game/gameobject.h"
#include "game/world.h"
#include "game/spatialindex.h"

BonusBehavior::BonusBehavior( GameObject & obj,
                              Behavior::Closure & )
              :obj(obj) {
  }

void BonusBehavior::tick(const Terrain &) {
  obj.world().spatial().visit( obj.x(), obj.y(),
                               obj.getClass().data.visionRange,
                               &BonusBehavior::lookOn,
                               obj );
  if( obj.hp()<=0 )
    obj.world().spatial().visit( obj.x(), obj.y(),
                                 obj.getClass().data.visionRange,
                                 &BonusBehavior::hillOn,
                                 obj );
  obj.rotate(3);
  //obj.setHP(0);
  }

void BonusBehavior::lookOn( GameObject &tg , GameObject &obj ){
  if( &tg==&obj )
    return;

  if( tg.behavior.find<BonusBehavior>() )
    return;

  if( tg.team()!=1 )
    return;

  int d  = tg.distanceSQ( obj.x(), obj.y() );
  int maxD = 2*Terrain::quadSize;
  maxD *= maxD;

  if( d<=maxD ){
    if( obj.hp()>0 ){
      obj.setHP(0);

      World& w = obj.world();
      w.emitHudAnim( "hud/move",
                     World::coordCast( obj.x() ),
                     World::coordCast( obj.y() ),
                     0.01 );
      }
    }
  }

void BonusBehavior::hillOn(GameObject &tg, GameObject &obj) {
  if( &tg==&obj )
    return;

  if( tg.behavior.find<BonusBehavior>() )
    return;

  if( tg.team()!=1 )
    return;

  int d  = tg.distanceSQ( obj.x(), obj.y() );
  int maxD = obj.getClass().data.visionRange*Terrain::quadSize;
  maxD *= maxD;

  if( d<=maxD ){
    int dhp = std::min( tg.getClass().data.maxHp-tg.hp(), 100);
    tg.setHP( tg.hp()+dhp );
    }
  }
