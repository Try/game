#include "spatialindex.h"

#include <algorithm>
#include "gameobject.h"

#include "landscape/terrain.h"

#include "behavior/movebehavior.h"

#include "world.h"
#include "util/math.h"

const int SpatialIndex::detail = 16;
const int SpatialIndex::qsize  = 16*Terrain::quadSize;

SpatialIndex::SpatialIndex(int iw, int ih):w(iw/detail), h(ih/detail) {
  psum.resize( w*h );
  obj.reserve( w*h );

  rndVec = 0;

  //clr.reserve( w*h );
  }

void SpatialIndex::fill(std::vector<PGameObject> &xobj ) {
  rndVec  = 0;
  sizeMax = 0;

  obj.resize( xobj.size() );

  std::fill( &psum[0], &psum[0]+psum.size(), 0 );
  std::fill( obj.begin(),  obj.end(),  (GameObject*)0 );
  //clr.clear();

  for( size_t i=0; i<xobj.size(); ++i ){
    int x = xobj[i]->x()/qsize;
    int y = xobj[i]->y()/qsize;

    sizeMax = std::max(sizeMax, xobj[i]->getClass().data.size );

    if( 0<=x && x<w &&
        0<=y && y<h ){
      size_t & v = psum[ x+y*w ];
      ++v;
      }
    }

  for( size_t i=1; i<psum.size(); ++i )
    psum[i] += psum[i-1];

  for( size_t i=1; i<psum.size(); ++i )
    psum[ psum.size()-i ] = psum[ psum.size()-i-1 ];
  psum[0] = 0;
  psum.push_back( xobj.size() );

  for( size_t i=0; i<xobj.size(); ++i ) {
    int x = xobj[i]->x()/qsize;
    int y = xobj[i]->y()/qsize;

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
      obj[i]->colisions.clear();
      solveColisions( obj[i], i );
      }
    }
  }

void SpatialIndex::solveColisions(GameObject * m, size_t /*id*/ ) {
  m->setColisionDisp(0,0);

  if( m->x() <= 0 )
    m->incColisionDisp( 20, 0 );

  if( m->x() >= m->world().terrain().width()*Terrain::quadSize )
    m->incColisionDisp( -20, 0 );

  if( m->y() <= 0 )
    m->incColisionDisp( 0, 20 );

  if( m->y() >= m->world().terrain().height()*Terrain::quadSize )
    m->incColisionDisp( 0, -20 );

  int x = m->x();
  int y = m->y();

  visit( x, y, 0, &collision, *m, rndVec );
  }

bool SpatialIndex::hasEffect(GameObject &tg, GameObject &obj) {
  if( obj.behavior.find<MoveBehavior>()==0 )
    return 0;

  if( tg.playerNum()!=obj.playerNum() )
    return 0;

  if( !tg.isOnMove() ){
    return 1;
    }

  if( !obj.isOnMove() ){
    return 0;
    }

  if( MoveBehavior *b1 = tg.behavior.find<MoveBehavior>() )
    if( MoveBehavior *b2 = obj.behavior.find<MoveBehavior>() ){
      return b1->isSameDirection(*b2);
      }

  return 0;
  }

void SpatialIndex::collision(GameObject &obj, GameObject &m,
                              int &id ) {
  int d = m.distanceSQ(obj);

  int maxD = (( m.getClass().data.size +
                obj.getClass().data.size )*Terrain::quadSize)/2;
  maxD = maxD*maxD;

  if( &m!=&obj && d <= 4*maxD ){
    obj.colisions.push_back(&m);
    m  .colisions.push_back(&obj);
    }


  if( (&m!=&obj) && hasEffect(m,obj) ){
    if( d <= maxD ){
      int dx = m.x() - obj.x();
      int dy = m.y() - obj.y();

      if( dx!=0 || dy!=0 ){
        int l = Math::distance(dx,dy,0,0);

        if( l>0 )
          m.incColisionDisp( 5*dx/l, 5*dy/l );
        } else {
        dx = Terrain::quadSize;
        dy = Terrain::quadSize;

        switch( id%4 ){
          case 0: m.incColisionDisp(  dx, dy ); break;
          case 1: m.incColisionDisp( -dx, dy ); break;
          case 2: m.incColisionDisp( -dx,-dy ); break;
          case 3: m.incColisionDisp(  dx,-dy ); break;
          }

        ++id;
        }
      }
    }
  }
