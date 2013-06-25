#ifndef ALGO_H
#define ALGO_H

#include <algorithm>
#include <iterator>

#include "util/array2d.h"

template< class C, class T >
auto find( C & c, const T & t ) -> decltype(c.begin()) {
  return std::find( c.begin(),
                    c.end(),
                    t );
  }

template< class C, class T >
bool contains( C & c, const T & t ) {
  return std::find( c.begin(),
                    c.end(),
                    t ) != c.end();
  }

template< class C, class T >
void remove( C & c, const T & t ){
  c.resize(  std::remove( c.begin(),
                          c.end(),
                          t) - c.begin() );
  }

template< class C, class F >
void remove_if( C & c, F f ){
  c.resize(
    std::remove_if( c.begin(),
                    c.end(),
                    f) - c.begin() );
  }


struct Point{
  int x, y;
  };

template< class T, class Ret, class ... Args >
struct MemberFunc{
  MemberFunc( T & t, Ret (T::*f)(Args...) ):object(t), func(f){}

  T & object;
  Ret (T::*func)(Args...);

  inline Ret operator() ( Args ... args ){
    return (object.*func)(args...);
    }
  };

template< class Val >
bool negTest( const array2d<Val> &  map,
              Point p,
              Point /*src*/ ){
  return map[p.x][p.y] < 0;
  }

template< class Val >
bool lessTest( const array2d<Val> &  map,
              Point p,
              Point src ){
  return map[p.x][p.y] < map[src.x][src.y];
  }

template< class Val >
void copyMapPoint( array2d<Val> &  map,
                   Point taget,
                   Point src ){
  map[taget.x][taget.y] = map[src.x][src.y];
  }

template< class Val >
void incMapPoint( array2d<Val> &  map,
                  const Point& taget,
                  const Point& src  ){
  map[taget.x][taget.y] = 1+map[src.x][src.y];
  }

template< class Val >
void incMapPointR( array2d<Val> &  map,
                   const Point& taget,
                   const Point& src ){
  if( abs(taget.x-src.x)+abs(taget.y-src.y) >= 2 )
    map[taget.x][taget.y] = 3+map[src.x][src.y]; else
    map[taget.x][taget.y] = 2+map[src.x][src.y];
  }

inline Point* dXY(){
  static Point d[8*2] =
  { {1, 0}, {-1, 0},
    {0, 1}, { 0,-1},
    {1, 1}, {-1, 1},
    {1,-1}, {-1,-1},

    {2, 0}, {-2, 0},
    {0, 2}, { 0,-2},
    {2, 2}, {-2, 2},
    {2,-2}, {-2,-2}};

  return d;
  }

inline bool trueFunc(){
  return true;
  }

inline bool falseFunc(){
  return false;
  }

template< class Val, class Test, class Edit, class TestForEnd >
void wave( array2d<Val> &  map, int x, int y, Val val,
           Test test, Edit edit, TestForEnd isEnd,
           int dimCount = 4 ) {
  std::vector<Point> v[2],
      *stk1 = &v[0],
      *stk2 = &v[1];

  Point p = {x,y};
  map[p.x][p.y] = val;
  stk1->push_back( p );

  //int dx[] = {1, -1, 0,  0 };
  //int dy[] = {0,  0, 1, -1 };
  Point * dxy = dXY();

  while( stk1->size() && !isEnd() ){
    for( size_t i=0; i<stk1->size(); ++i ){
      for( int r=0; r<dimCount; ++r ){
        Point p  = (*stk1)[i];
        p.x += dxy[r].x;
        p.y += dxy[r].y;

        if( 0<= p.x && p.x < map.width() &&
            0<= p.y && p.y < map.height() ){
          if( test( map,  p, (*stk1)[i] )   ){
            edit( map, p, (*stk1)[i] );
            stk2->push_back( p );
            }
          }
        }
      }

    std::swap( stk1, stk2 );
    stk2->clear();
    }

  }

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


template< class T >
T sgn( const T& v ){
  if( v<0 )
    return -1;

  if( v==0 )
    return 0;

  return 1;
  }

template< class T >
T clamp( const T& v, const T& a, const T& b ){
  if( v<a )
    return a;

  if( v>b )
    return b;

  return v;
  }

#endif // ALGO_H
