#include "spatialindex.h"

#include <algorithm>
#include "gameobject.h"

#include "landscape/terrain.h"

#include "behavior/movebehavior.h"

SpatialIndex::SpatialIndex(int w, int h):w(w), h(h) {
  psum.resize( w*h );
  }

void SpatialIndex::fill(std::vector<PGameObject> &xobj ) {
  sizeMax = 0;

  obj.resize( xobj.size() );

  std::fill( psum.begin(), psum.end(), 0 );
  std::fill( obj.begin(),  obj.end(),  (GameObject*)0 );

  for( size_t i=0; i<xobj.size(); ++i )
    if( xobj[i]->behavior.find<MoveBehavior>() ){
      int x = xobj[i]->x()/Terrain::quadSize;
      int y = xobj[i]->y()/Terrain::quadSize;

      sizeMax = std::max(sizeMax, xobj[i]->getClass().data.size );

      if( 0<=x && x<w &&
          0<=y && y<h ){
        ++psum[ x+y*w ];
        }
      }

  for( size_t i=1; i<psum.size(); ++i )
    psum[i] += psum[i-1];

  for( size_t i=1; i<psum.size(); ++i )
    psum[ psum.size()-i ] = psum[ psum.size()-i-1 ];
  psum[0] = 0;
  psum.push_back( xobj.size() );

  for( size_t i=0; i<xobj.size(); ++i )
    if( xobj[i]->behavior.find<MoveBehavior>() ){
      int x = xobj[i]->x()/Terrain::quadSize;
      int y = xobj[i]->y()/Terrain::quadSize;

      if( 0<=x && x<w &&
          0<=y && y<h ){
        size_t &id = psum[ x+y*w ];
        obj[id] = xobj[i].get();
        ++id;
        }
      }

  for( size_t i=1; i<psum.size(); ++i )
    psum[ psum.size()-i ] = psum[ psum.size()-i-1 ];
  psum[0] = 0;
  }

void SpatialIndex::clear() {

  }

void SpatialIndex::solveColisions() {
  for( size_t i=0; i<obj.size(); ++i ){
    if( obj[i] && obj[i]->isMoviable() && !obj[i]->isMineralMove() ){
      solveColisions( obj[i], i );
      }
    }
  }

void SpatialIndex::solveColisions( GameObject * m, size_t id ) {
  m->setColisionDisp(0,0);

  int x = m->x()/Terrain::quadSize;
  int y = m->y()/Terrain::quadSize;

  int lx = x - sizeMax,
      rx = x + sizeMax;

  int ly = y - sizeMax,
      ry = y + sizeMax;

  lx = std::max(0, lx);
  rx = std::min(w, rx);

  ly = std::max(0, ly);
  ry = std::min(h, ry);

  for( int y=ly; y<ry; ++y ){
    int l = psum[ lx+y*w ],
        r = psum[ rx+y*w ];

    for( int i=l; i<r; ++i ){
      GameObject *obj2 = obj[i];

      int d = m->distanceSQ(*obj2);
      int maxD = ( m->getClass().data.size    *Terrain::quadSize +
                   obj2->getClass().data.size *Terrain::quadSize)/2;
      maxD = maxD*maxD;

      if( d <= maxD && m!=obj2 && hasEffect(m,obj2) ){
        int dx = m->x() - obj2->x();
        int dy = m->y() - obj2->y();

        if( dx!=0 || dy!=0 ){
          m->incColisionDisp( dx, dy );
          } else {
          dx = Terrain::quadSize;
          dy = Terrain::quadSize;

          switch( id%4 ){
            case 0: m->incColisionDisp(  dx, dy ); break;
            case 1: m->incColisionDisp( -dx, dy ); break;
            case 2: m->incColisionDisp( -dx,-dy ); break;
            case 3: m->incColisionDisp(  dx,-dy ); break;
            }
          }
        }
      }
    }
  }

bool SpatialIndex::hasEffect(GameObject *tg, GameObject *obj) {
  if( tg->playerNum()!=obj->playerNum() )
    return 1;

  if( !tg->isOnMove() ){
    return 1;
    }

  if( !obj->isOnMove() ){
    return 0;
    }

  if( MoveBehavior *b1 = tg->behavior.find<MoveBehavior>() )
    if( MoveBehavior *b2 = obj->behavior.find<MoveBehavior>() ){
      return b1->isSameDirection(*b2);
      }

  return 0;
  }
