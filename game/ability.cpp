#include "ability.h"

#include "world.h"
#include "util/math.h"

#include "player.h"
#include "game.h"

#include "bullet.h"
#include "behavior/buildingbehavior.h"

Ability::Ability() {
  }

bool Ability::spell( Game &g,
                     World &w,
                     GameObject &obj,
                     const BehaviorMSGQueue::MSG &m ) {
  if( m.str=="heal" ){
    return heal(g,w,obj,m);
    }

  if( m.str=="blink" ){
    return blink(g,w,obj,m);
    }

  if( m.str=="fire_strike" ){
    return fireStrike(g,w,obj,m);
    }

  if( m.str=="fire_storm" ){
    return fireStorm(g,w,obj,m);
    }

  if( m.str=="ice_bolt" ){
    return iceBolt(g,w,obj,m);
    }

  if( m.str=="force_field" ){
    return forceField(g,w,obj,m);
    }

  return 0;
  }

void Ability::autoCast( Game &g, World &w,
                        const std::string &spell,
                        GameObject & obj ) {
  if( spell=="fire_storm" ){
    const Spell & s = g.prototypes().spell(spell);
    GameObject *tg = 0;
    int r = 8, rq = r*Terrain::quadSize;
    int x = obj.x(), y = obj.y(), team = obj.team();

    w.spatial().visit( x, y,
                       r,
                       &findEnemy,
                       x,
                       y,
                       rq,
                       team,
                       tg );

    if( tg && obj.coolDown( s.id )==0 )
      fireStorm( w, obj, tg->x(), tg->y(), s);
    }

  if( spell=="fire_strike" ){
    const Spell & s = g.prototypes().spell(spell);
    GameObject *tg = 0;
    int r = 8, rq = r*Terrain::quadSize;
    int x = obj.x(), y = obj.y(), team = obj.team();

    w.spatial().visit( x, y,
                       r,
                       &findEnemy,
                       x,
                       y,
                       rq,
                       team,
                       tg );

    if( tg && obj.coolDown( s.id )==0 )
      fireStrike( w, obj, *tg, s);
    }

  if( spell=="heal" ){
    const Spell & s = g.prototypes().spell(spell);
    GameObject *tg = 0;
    int r = 6, rq = r*Terrain::quadSize;
    int x = obj.x(), y = obj.y();

    w.spatial().visit( x, y,
                       r,
                       &findAlly,
                       x,
                       y,
                       rq,
                       obj,
                       tg );

    if( tg && obj.coolDown( s.id )==0 )
      heal( w, obj, *tg, s);
    }
  }

bool Ability::blink( Game &g,
                     World &w,
                     GameObject &obj,
                     const BehaviorMSGQueue::MSG &m ) {
  const Spell & s = g.prototypes().spell(m.str);

  int dx = m.x - obj.x(),
      dy = m.y - obj.y();
  int lx = 8*Terrain::quadSize;

  while( abs(dx) + abs(dy) > lx*2 ){
    dx /= 2;
    dy /= 2;
    }

  int l = Math::distance( dx, dy, 0, 0 );

  if( l>lx ){
    dx = dx*lx/l;
    dy = dy*lx/l;
    }

  std::vector<GameObject*> & objs = obj.player().selected();
  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & u = *objs[i];

    int cd = u.coolDown( s.id );
    if( cd==0 ){
      u.setCoolDown( s.id, s.coolDown );

      int x = u.x() + dx,
          y = u.y() + dy;

      Terrain & t = w.terrain();
      float wx = x/Terrain::quadSizef,
            wy = y/Terrain::quadSizef;

      w.emitHudAnim( "hud/blink",
                     u.x(),
                     u.y(),
                     0.01 );
      w.emitHudAnim( "hud/blink",
                     x,
                     y,
                     0.01 );

      u.setPosition( x, y, t.heightAt(wx,wy) );
      u.behavior.message( BehaviorMSGQueue::Cancel, 0, 0 );
      }
    }

  return 1;
  }

bool Ability::fireStrike( Game &g,
                          World &w,
                          GameObject &obj,
                          const BehaviorMSGQueue::MSG &m) {
  if( m.size == size_t(-1) )
    return 0;

  GameObject & tg = w.object( m.size );

  const Spell & s = g.prototypes().spell("fire_strike");

  std::vector<GameObject*> & objs = obj.player().selected();

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & u = *objs[i];

    int cd = u.coolDown( s.id );
    if( cd==0 && fireStrike(w, u,tg, s) )
      return 1;
    }

  return 1;
  }

bool Ability::fireStrike( World &w,
                          GameObject &u,
                          GameObject &tg,
                          const Spell & s ) {
  int cd = u.coolDown( s.id );
  if( cd==0 && !tg.behavior.find<BuildingBehavior>() ){
    u.setCoolDown( s.id, s.coolDown );

    w.emitHudAnim( "hud/blink",
                   u.x(),
                   u.y(),
                   0.01 );

    w.emitHudAnim( "smoke",
                   tg.x(),
                   tg.y(),
                   0.01 );

    w.emitHudAnim( "fire",
                   tg.x(),
                   tg.y(),
                   0.01 );

    auto bul = tg.reciveBulldet( "bullets/fire_large" );
    Bullet& b = *bul;

    b.x = u.x();
    b.y = u.y();
    b.setTeamColor( u.teamColor() );

    b.z   = u.viewHeight()/2  + World::coordCast(u.z());
    b.tgZ = tg.viewHeight()/2 + World::coordCast(tg.z());

    b.speed        = s.bulletSpeed;
    b.atack.damage = 150;
    b.tick();
    return 1;
    }

  return 0;
  }


bool Ability::heal( Game &g,
                    World &w,
                    GameObject &obj,
                    const BehaviorMSGQueue::MSG &m) {
  if( m.size == size_t(-1) )
    return 0;

  GameObject & tg = w.object( m.size );

  const Spell & s = g.prototypes().spell("heal");

  std::vector<GameObject*> & objs = obj.player().selected();

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & u = *objs[i];

    int cd = u.coolDown( s.id );
    if( cd==0 && heal(w, u,tg, s) )
      return 1;
    }

  return 1;
  }

bool Ability::heal( World &w,
                    GameObject &u,
                    GameObject &tg,
                    const Spell & s ) {
  int cd = u.coolDown( s.id );
  if( cd==0 && !tg.behavior.find<BuildingBehavior>() &&
      &u!=&tg &&
      tg.hp() != tg.getClass().data.maxHp ){
    u.setCoolDown( s.id, s.coolDown );

    /*
    w.emitHudAnim( "hud/blink",
                   ( u.x() ),
                   ( u.y() ),
                   0.01 );
    */

    w.emitHudAnim( "heal",
                   tg.x(),
                   tg.y(),
                   0.01 );

    tg.setHP( std::min(tg.hp()+150, tg.getClass().data.maxHp) );
    return 1;
    }

  return 0;
  }

bool Ability::fireStorm( Game  &g,
                         World &w,
                         GameObject &obj,
                         const BehaviorMSGQueue::MSG &m ) {
  const Spell & s = g.prototypes().spell("fire_storm");

  std::vector<GameObject*> & objs = obj.player().selected();

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & u = *objs[i];

    int cd = u.coolDown( s.id );
    if( cd==0 && fireStorm(w, u, m.x, m.y, s) )
      return 1;
    }

  return 1;
  }

bool Ability::fireStorm( World &w,
                         GameObject &u,
                         int tgX,
                         int tgY,
                         const Spell &s) {
  int cd = u.coolDown( s.id );
  if( cd==0 ){
    u.setCoolDown( s.id, s.coolDown );

    w.emitEfect<StormEfect>( "storm" ).setPosition(tgX,tgY,0);
    w.emitHudAnim( "storm",
                   tgX,
                   tgY,
                   0.01 );

    return 1;
    }

  return 0;
  }

void Ability::findEnemy( GameObject &tg,
                         int x, int y, int r,
                         int team, GameObject *&out) {
  int d  = tg.distanceSQ( x, y );

  if( tg.team() == team )
    return;

  if( d<r*r && !tg.behavior.find<BuildingBehavior>())
    out = &tg;
  }

void Ability::findAlly( GameObject &tg,
                        int x, int y, int r,
                        GameObject &caster,
                        GameObject *&out) {
  int d  = tg.distanceSQ( x, y );

  if( tg.team() != caster.team() )
    return;

  if( d<r*r && &tg!=&caster &&
      ( tg.hp() < tg.getClass().data.maxHp-150 || tg.hp()<tg.getClass().data.maxHp/3 ) &&
      !tg.behavior.find<BuildingBehavior>() )
    out = &tg;
  }


bool Ability::forceField( Game  &g,
                         World &w,
                         GameObject &obj,
                         const BehaviorMSGQueue::MSG &m ) {
  const Spell & s = g.prototypes().spell("force_field");

  std::vector<GameObject*> & objs = obj.player().selected();

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & u = *objs[i];

    int cd = u.coolDown( s.id );
    if( cd==0 && forceField(w, u, m.x, m.y, s) )
      return 1;
    }

  return 1;
  }

bool Ability::forceField( World &w,
                         GameObject &u,
                         int tgX,
                         int tgY,
                         const Spell &s) {
  int cd = u.coolDown( s.id );
  if( cd==0 ){
    u.setCoolDown( s.id, s.coolDown );

    w.emitEfect<ForceFieldEfect>( "bullets/ice" ).setPosition(tgX,tgY,0);
    /*
    w.emitHudAnim( "storm",
                   tgX,
                   tgY,
                   0.01 );*/

    return 1;
    }

  return 0;
  }


bool Ability::iceBolt( Game &g,
                       World &w,
                       GameObject &obj,
                       const BehaviorMSGQueue::MSG &m) {
  if( m.size == size_t(-1) )
    return 0;

  GameObject & tg = w.object( m.size );

  const Spell & s = g.prototypes().spell("ice_bolt");

  std::vector<GameObject*> & objs = obj.player().selected();

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & u = *objs[i];

    int cd = u.coolDown( s.id );
    if( cd==0 && iceBolt(w, u,tg, s) )
      return 1;
    }

  return 1;
  }

bool Ability::iceBolt( World &w,
                       GameObject &u,
                       GameObject &tg,
                       const Spell & s ) {
  int cd = u.coolDown( s.id );
  if( cd==0 && !tg.behavior.find<BuildingBehavior>() ){
    u.setCoolDown( s.id, s.coolDown );

    auto bul = tg.reciveBulldet( "bullets/ice_large" );
    Bullet& b = *bul;

    w.emitHudAnim( "blue_gaizer",
                   tg.x(),
                   tg.y(),
                   0.01 );
    b.x = u.x();
    b.y = u.y();
    b.setTeamColor( u.teamColor() );

    b.z   = u.viewHeight()/2  + World::coordCast(u.z());
    b.tgZ = tg.viewHeight()/2 + World::coordCast(tg.z());

    b.speed        = s.bulletSpeed;
    b.atack.damage = 75;
    b.tick();
    return 1;
    }

  return 0;
  }
