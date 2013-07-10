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

  return 0;
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
                     World::coordCast( u.x() ),
                     World::coordCast( u.y() ),
                     0.01 );
      w.emitHudAnim( "hud/blink",
                     World::coordCast( x ),
                     World::coordCast( y ),
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

  const Spell & s = g.prototypes().spell(m.str);

  std::vector<GameObject*> & objs = obj.player().selected();

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & u = *objs[i];

    int cd = u.coolDown( s.id );
    if( cd==0 && !tg.behavior.find<BuildingBehavior>() ){
      u.setCoolDown( s.id, s.coolDown );

      w.emitHudAnim( "hud/blink",
                     World::coordCast( u.x() ),
                     World::coordCast( u.y() ),
                     0.01 );

      w.emitHudAnim( "smoke",
                     World::coordCast( tg.x() ),
                     World::coordCast( tg.y() ),
                     0.01 );

      w.emitHudAnim( "fire",
                     World::coordCast( tg.x() ),
                     World::coordCast( tg.y() ),
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
    }

  return 1;
  }

bool Ability::heal( Game &g,
                    World &w,
                    GameObject &obj,
                    const BehaviorMSGQueue::MSG &m) {
  if( m.size == size_t(-1) )
    return 0;

  GameObject & tg = w.object( m.size );

  const Spell & s = g.prototypes().spell(m.str);

  std::vector<GameObject*> & objs = obj.player().selected();

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & u = *objs[i];

    int cd = u.coolDown( s.id );
    if( cd==0 ){
      u.setCoolDown( s.id, s.coolDown );

      w.emitHudAnim( "hud/blink",
                     World::coordCast( u.x() ),
                     World::coordCast( u.y() ),
                     0.01 );

      auto bul = tg.reciveBulldet( "bullets/fire_large" );
      Bullet& b = *bul;

      b.x = u.x();
      b.y = u.y();
      b.setTeamColor( u.teamColor() );

      b.z   = u.viewHeight()/2  + World::coordCast(u.z());
      b.tgZ = tg.viewHeight()/2 + World::coordCast(tg.z());

      b.atack.damage = 150;
      b.speed  = 250;
      b.tick();

      u.setHP( std::min(u.getClass().data.maxHp, u.hp()+10) );
      return 1;
      }
    }

  return 1;
  }
