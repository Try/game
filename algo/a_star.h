#ifndef A_STAR_H
#define A_STAR_H

#include "algo.h"

struct Pt:Point{
  int f;

  void mkF(int rx, int ry, int v ){
    //f = v;//4*(abs(x-rx)+abs(y-ry))+v;
    rx = abs(rx-x);
    ry = abs(ry-y);
    if( rx<ry ){
      f = v + rx*3 + (ry-rx)*2;
      } else {
      f = v + ry*3 + (rx-ry)*2;
      }
    }

  bool operator < ( const Pt& x ) const{
    return f > x.f;
    }
  };

template< class Val, class Test, class Edit >
void waveAstar( array2d<Val> &  map,
                //const array2d<Val> &  evMap,
                int x, int y,
                Test test, Edit edit,
                int rx, int ry,
                std::vector<Point> & rwPoint,
                std::vector<Pt>    & stk,
                int dimCount = 4 ) {
  //std::vector<Pt> stk;

  Pt p;// = {x,y, 1};
  p.x = x;
  p.y = y;
  p.mkF(rx,ry,1);

  map[p.x][p.y] = 1;

  stk.resize(1);
  rwPoint.resize(1);
  stk[0]     = p;
  rwPoint[0] = p;

  //int dx[] = {1, -1, 0,  0 };
  //int dy[] = {0,  0, 1, -1 };
  Point * dxy = dXY();

  while( stk.size() && map[rx][ry]==-1 ){
    Pt px = stk.back();
    stk.pop_back();

    for( int r=0; r<dimCount; ++r ){
      Pt p  = px;
      p.x += dxy[r].x;
      p.y += dxy[r].y;

      if( 0<= p.x && p.x < map.width() &&
          0<= p.y && p.y < map.height() ){
        if( test( map,  p, px )   ){
          edit( map, p, px );
          p.mkF(rx, ry, map[p.x][p.y] );
          //p.f += evMap[p.x][p.y];

          size_t l = std::lower_bound( stk.begin(), stk.end(), p ) - stk.begin();
          stk.insert( stk.begin()+l, p );
          rwPoint.push_back(p);
          }
        }
      }

    //std::swap( stk1, stk2 );
    //stk2->clear();
    }

  }

#endif // A_STAR_H
