#include "wayfindalgo.h"

#include "landscape/terrain.h"
#include "game/gameobject.h"
#include "behavior/movebehavior.h"

#include "algo/algo.h"
#include "algo/a_star.h"

#include <Tempest/signal>

#include <fstream>

//array2d<int> WayFindAlgo::clasterMap, WayFindAlgo::wayMap;

WayFindAlgo::WayFindAlgo( const Terrain &t ) : terrain(t){
  clasterMap.resize( t.width() -1,
                     t.height()-1 );
  wayMap.resize( t.width()-1, t.height()-1 );

  std::fill( wayMap.begin(), wayMap.end(), -1 );
  std::fill( clasterMap.begin(), clasterMap.end(), 0 );

  waveBuf.reserve(1024);
  rwPoint.reserve(1024);
  way.reserve(1024);
  wayBuf.reserve(1024);
  }

void WayFindAlgo::fillClasrerMap( const std::vector< GameObject* > &objects ) {
  std::fill( clasterMap.begin(), clasterMap.end(), 0 );

  for( size_t i=0; i<objects.size(); ++i ){
    GameObject & obj = *objects[i];

    int x = obj.x()/Terrain::quadSize,
        y = obj.y()/Terrain::quadSize,
        sz = obj.getClass().data.size,
        szd2 = sz/2;

    for( int ix=0; ix<sz; ++ix )
      for( int iy=0; iy<sz; ++iy )
        if( clasterMap.validate( x-szd2+ix, y-szd2+iy ) )
          clasterMap[x-szd2+ix][y-szd2+iy] = -1;
    }

  clasterNum = 1;
  for( size_t i=0; i<objects.size(); ++i ){
    GameObject & obj = *objects[i];

    int x = obj.x()/Terrain::quadSize,
        y = obj.y()/Terrain::quadSize;

    if( clasterMap.validate(x,y) && clasterMap[x][y]== -1 ){
      fill( x, y, clasterNum );
      ++clasterNum;
      }
    }

  }

void WayFindAlgo::findWay( std::vector< GameObject* >& objs,
                           int rx, int ry ) {
  rx /= Terrain::quadSize;
  ry /= Terrain::quadSize;

  std::vector< std::shared_ptr< std::vector<Point> > > ways( objs.size() );

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & obj = *objs[i];
    MoveBehavior *bobj = obj.behavior.find<MoveBehavior>();

    int x = obj.x()/Terrain::quadSize,
        y = obj.y()/Terrain::quadSize;

    if( !ways[i] ){
      findWay( obj, x, y, rx, ry );
      if( bobj ){
        bobj->setWay( way );
        }
      ways[i] = std::make_shared<std::vector<Point> >( way );
      setupWaysGroup( obj, ways[i], objs, ways );
      }

    }
  }

void WayFindAlgo::setupWaysGroup( GameObject & obj,
                                  std::shared_ptr<std::vector<Point> >& way,
                                  std::vector<GameObject*> &objs,
                                  std::vector<std::shared_ptr<std::vector<Point> > > &w) {
  std::vector< GameObject* > stk[2], *stk1, *stk2;
  stk[0].push_back(&obj);

  stk1 = &stk[0];
  stk2 = &stk[1];

  while( stk1->size() ){
    for( size_t i=0; i<stk1->size(); ++i ){
      GameObject & ob = *stk1->at(i);
      for( size_t r=0; r<ob.colisions.size(); ++r )
        for( size_t q=0; q<objs.size(); ++q ){
          if( objs[q]==ob.colisions[r] && !w[q] ){
            w[q] = way;
            MoveBehavior *bobj = ob.colisions[r]->behavior.find<MoveBehavior>();
            if( bobj )
              bobj->setWay( *way );

            stk2->push_back( objs[q] );
            }
          }
      }

    std::swap(stk1, stk2);
    stk2->clear();
    }
  }

void WayFindAlgo::dump() {
  std::ofstream fout("./debug.txt");

  for( int i=0; i<wayMap.width(); ++i ){
    for( int r=0; r<wayMap.height(); ++r ){
      char x = ' ';
      bool v = terrain.isEnable(i,r);

      if( wayMap.at(i,r)<10 )
        x = '0' + wayMap.at(i,r);

      if( !v )
        x = 'x';

      for( size_t q=0; q<way.size(); ++q )
        if( way[q].x==i && way[q].y==r ){
          if( terrain.isEnableQuad(i-1,r, 3) )
            x = '*'; else
            x = '@';
          }

      fout << x;
      }

    fout << std::endl;
    }

  fout.close();
  }

void WayFindAlgo::fill( int x, int y, int val ) {
  wave( clasterMap, x, y, val,
        negTest<int>,
        copyMapPoint<int>,
        falseFunc,
        16 );
  }

void WayFindAlgo::findWay(std::vector<GameObject*> &objs,
                          GameObject & obj,
                          int x,
                          int y,
                          int rx,
                          int ry,
                          int cls) {
  auto fix = terrain.nearestEnable( rx, ry, obj.x(), obj.y(), 1 );
  rx = fix.first;
  ry = fix.second;

  if( !wayMap.validate(rx,ry) )
    return;

  findWay( obj, x, y, rx, ry );

  for( size_t i=0; i<objs.size(); ++i ){
    GameObject & obj = *objs[i];
    MoveBehavior * b = obj.behavior.find<MoveBehavior>();

    int x = obj.x()/Terrain::quadSize,
        y = obj.y()/Terrain::quadSize;

    if( b && clasterMap.validate(x,y) && clasterMap[x][y]==cls ){
      b->setWay( way );
      }
    }
  }

void WayFindAlgo::findWay( GameObject & obj, int x, int y, int rx, int ry ) {
  findWay(obj,x,y,rx,ry, 0);
  }

void WayFindAlgo::findWay( GameObject & obj, int x, int y, int rx, int ry,
                           int ref ) {
  vMapRef = std::min(9, ref);
  way.clear();

  auto fix = terrain.nearestEnable( rx, ry,
                                    obj.x(), obj.y(),
                                    1 );
  rx = fix.first;
  ry = fix.second;

  if( !wayMap.validate(rx,ry) )
    return;

  fix = terrain.nearestEnable( x, y, 1 );
  x = fix.first;
  y = fix.second;

  rPointX = rx;
  rPointY = ry;

  if( abs(x-rx) + abs(y-ry) < 5 &&
      optimizeWay( Point{x,y}, Point{rx,ry} ) ){
    way.push_back(Point{rx,ry});
    way.push_back(Point{ x, y});
    return;
    }

  //std::fill( wayMap.begin(), wayMap.end(), -1 );

  MemberFunc< WayFindAlgo, bool,
              const array2d<int> &, Point, Point>
      func(*this, &WayFindAlgo::isQuadEnable);

  int dimCount = 8;
  waveAstar( wayMap,
             //terrain.wayCorrMap(),
             x, y,
             func, incMapPointR<int>,
             rx, ry,
             rwPoint,
             waveBuf,
             dimCount );

  //dump();

  if( wayMap.at(rx,ry)==-1 ){
    for( size_t i=0; i<rwPoint.size(); ++i ){
      Point p = rwPoint[i];
      wayMap[p.x][p.y] = -1;
      }
    return;
    }

  Point re = {rx, ry};
  Point* dxy = dXY();

  way.clear();
  while( (re.x!=x || re.y!=y) ){
    bool found = 0;
    int maxR = -1;

    for( int i=0; i<dimCount; ++i ){
      Point p = {re.x+dxy[i].x, re.y+dxy[i].y };

      int pVal = 0;
      if( wayMap.validate(p.x, p.y) )
        pVal = wayMap[ p.x][ p.y];

      if( wayMap.validate(p.x, p.y) &&
          terrain.isEnable( p.x, p.y ) &&//isQuadEnable( p, re) &&
          pVal > 0 &&
          pVal < wayMap[re.x][re.y] ){
        maxR = std::max( maxR, wayMap[re.x][re.y]-pVal );
        //break;
        }
      }

    for( int i=0; i<dimCount; ++i ){
      Point p = {re.x+dxy[i].x, re.y+dxy[i].y };

      int pVal = 0;
      if( wayMap.validate(p.x, p.y) )
        pVal  = wayMap[ p.x][ p.y];

      if( wayMap.validate(p.x, p.y) &&
          terrain.isEnable( p.x, p.y ) &&//isQuadEnable( p, re) &&
          pVal > 0 &&
          wayMap[re.x][re.y]-pVal==maxR ){
        way.push_back(re);
        re = p;
        found = true;
        //break;
        }
      }

    assert( found );
    }

  way.push_back(re);
  //dump();
  optimizeWay();

  for( size_t i=0; i<rwPoint.size(); ++i ){
    Point p = rwPoint[i];
    wayMap[p.x][p.y] = -1;
    }
  //dump();
  }

bool WayFindAlgo::isQuadEnable( Point p,
                                Point s ) {
  //int v1 = map[s.x][s.y];

  return terrain.isEnable( p.x, p.y ) &&
         terrain.isEnable( s.x, p.y ) &&
         terrain.isEnable( p.x, s.y );
  }

bool WayFindAlgo::isQuadEnable( const array2d<int> &  map,
                                Point p,
                                Point s ) {
  int v0 = map[p.x][p.y];
  //int v1 = map[s.x][s.y];

  return ( v0<0 ) &&
         //terrain.wayCorrMap(p.x, p.y) >= vMapRef &&
         terrain.isEnable( p.x, p.y ) &&
         terrain.isEnable( s.x, p.y ) &&
         terrain.isEnable( p.x, s.y );
  }

void WayFindAlgo::optimizeWay() {
  //return;

  if( way.size()==0 )
    return;

  int s = 0;
  int e = 2, wr = 1;
  while( e < int(way.size()) ){
    if( !optimizeWay( way[s], way[e] ) || e-s>20 ){
      way[wr] = way[e-1];
      ++wr;
      s  = e-1;
      e  = s+2;
      } else {
      ++e;
      }
    }

  if( wr+1 <= int(way.size()) ){
    Point l = way.back();
    way.resize(wr+1);
    way[wr] = l;

    if( way.size()>=2 &&
        way[way.size()-1].x==way[way.size()-2].x &&
        way[way.size()-1].y==way[way.size()-2].y )
      way.pop_back();
    }


  wayBuf.clear();
  if( way.size() )
    wayBuf.push_back(way[0]);

  for( size_t i=1; i<way.size(); ++i ){
    const Point &a = way[i-1],
                &b = way[i];
    int dx = a.x-b.x, dy = a.y-b.y, c = 0;
    while( abs(dx)+abs(dy) > Terrain::quadSize*10 ){
      dx /= 2;
      dy /= 2;
      ++c;
      }

    dx = a.x-b.x;
    dy = a.y-b.y;
    ++c;
    for( int i=1; i<c; ++i ){
      wayBuf.push_back( Point{a.x+dx*i/c, a.y+dy*i/c} );
      }
    wayBuf.push_back(b);
    }

  way = wayBuf;
  }

bool WayFindAlgo::optimizeWay( const Point& a, const Point& b) {
  //return 0;

  if( a.x==b.x ){
    int y0 = std::min(a.y, b.y),
        y1 = std::max(a.y, b.y);
    for( int i=y0; i<y1; ++i )
      if( !terrain.isEnableQuad(a.x, i, 1) )
        return false;

    return true;
    }
  if( a.y==b.y ){
    int x0 = std::min(a.x, b.x),
        x1 = std::max(a.x, b.x);
    for( int i=x0; i<x1; ++i )
      if( !terrain.isEnableQuad(i, a.y, 1) )
        return false;

    return true;
    }

  if( !(/*terrain.isEnableQuad(a.x, a.y, 3) &&*/ terrain.isEnableQuad(b.x, b.y, 3)) )
    return false;

  int x1 = a.x, y1 = a.y, x2 = b.x, y2 = b.y,
      x = x1, y = y1;
  const int deltaX = abs(x2 - x1);
  const int deltaY = abs(y2 - y1);
  const int signX = x1 < x2 ? 1 : -1;
  const int signY = y1 < y2 ? 1 : -1;

  int error = deltaX - deltaY;

  while(x1 != x2 || y1 != y2) {
    if( !( terrain.isEnable(x1, y1) &&
           terrain.isEnable(x,  y1) &&
           terrain.isEnable(x1, y ) &&
           terrain.isEnable(x,  y ) ) )
      return false;
    //if( !terrain.isEnableQuad(x1, y1, 2) )
      //return false;
    x = x1;
    y = y1;

    const int error2 = error * 2;
    //
    if(error2 > -deltaY) {
      error -= deltaY;
      x1 += signX;
    }
    if(error2 < deltaX) {
      error += deltaX;
      y1 += signY;
      }
    }

  return true;
  }

bool WayFindAlgo::isRPoint() {
  int v = wayMap.at( rPointX, rPointY );
  return v != -1;
  }
