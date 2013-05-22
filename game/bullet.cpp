#include "bullet.h"

#include "world.h"

#include "util/math.h"

#include <cmath>

Bullet::Bullet( Scene & s,
                World & wrld,
                const ProtoObject &p,
                const PrototypesLoader & pl )
       :view(s, wrld, p, pl) {
  x = 0;
  y = 0;
  z = 0;

  tgX = 0;
  tgY = 0;
  tgZ = 0;

  l0 = 0;

  isFinished = 0;
  absDmg     = 0;

  speed = 600;

  std::fill(mvec, mvec+3, 0);
  }

void Bullet::tick() {
  view.setPosition(x,y);

  int vecX = tgX - x,
      vecY = tgY - y;

  int l = Math::distance(vecX, vecY, 0,0);

  if( l0==0 ){
    l0 = l;
    view.setRotation( atan2( (double)vecY, (double)vecX)*180.0/M_PI );
    }

  mvec[2] = 0;
  view.setViewPosition( World::coordCast(x),
                        World::coordCast(y),
                        tgZ + (z-tgZ)*l/l0 );

  if( l <= speed ){
    isFinished = 1;
    return;
    }

  vecX *= speed;
  vecY *= speed;

  vecX /= l;
  vecY /= l;

  x += vecX;
  y += vecY;

  mvec[0] = 0.015*vecX;
  mvec[1] = 0.015*vecY;
  view.setViewDirection( vecX, vecY );
  view.tick();
  }
