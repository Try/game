#include "bonusbehavior.h"

#include "game/gameobject.h"
#include "game/world.h"
#include "game/spatialindex.h"

#include "game.h"
#include "game/scenario.h"

BonusBehavior::BonusBehavior( GameObject & obj,
                              Behavior::Closure & )
              :obj(obj) {
  }

void BonusBehavior::tick(const Terrain &) {
  obj.world().spatial().visit( obj.x(), obj.y(),
                               obj.getClass().data.visionRange,
                               &BonusBehavior::lookOn,
                               obj );

  if( obj.hp()<=0 ){
    if( obj.getClass().data.propStr("bonusType")=="hill" ){
      obj.world().spatial().visit( obj.x(), obj.y(),
                                   obj.getClass().data.visionRange,
                                   &BonusBehavior::hillOn,
                                   obj );
      }

    if( obj.getClass().data.propStr("bonusType")=="event" ){
      obj.game().scenario().onItemEvent( obj );
      }
    }

  if( obj.getClass().data.propStr("bonusAnim")=="rotate"  )
    obj.rotate(3);

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
      w.emitHudAnim( "hud/take",
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
