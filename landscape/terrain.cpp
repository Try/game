#include "terrain.h"

#include <MyGL/Model>

#include "game/world.h"

#include <cmath>
#include "algo/algo.h"
#include "util/gameserializer.h"
#include <memory>

const int Terrain::busyMapsCount = 4;

Terrain::Terrain(int w, int h) {
  groupMask = 0;

  heightMap.resize( w+1, h+1 );
  waterMap .resize( w+1, h+1 );
  buildingsMap.resize(w+1, h+1);

  for( int i=0; i<busyMapsCount; ++i ){
    int n = i+1;
    busyMap[i].resize(w/n,h/n);
    }

  std::fill( heightMap.begin(),    heightMap.end(),     0 );
  std::fill( waterMap.begin(),     waterMap.end(),      0 );
  std::fill( buildingsMap.begin(), buildingsMap.end(),  0 );
  resetBusyMap();
  computeEnableMap();
  }

MyGL::Model<> Terrain::buildGeometry( MyGL::VertexBufferHolder & vboHolder,
                                      MyGL::IndexBufferHolder  & iboHolder ) const {
  MyGL::Model<> model;
  MyGL::Model<>::Vertex v = {0,0,0, 0,0, {0,0,1}};
  std::vector< MyGL::Model<>::Vertex > land;
  const int dx[] = {0, 1, 1, 0, 1, 0},
            dy[] = {0, 0, 1, 0, 1, 1};

  const double texCoordScale = 0.1;

  for( int i=0; i+1<heightMap.width(); ++i )
    for( int r=0; r+1<heightMap.height(); ++r )
      for( int q=0; q<6; ++q ){
        int x = (i+dx[q]),
            y = (r+dy[q]);
        v.x = World::coordCast( x*quadSize );
        v.y = World::coordCast( y*quadSize );

        int dz = waterMap[ i+dx[q] ][ r+dy[q] ];
        v.z = World::coordCast( heightMap[ i+dx[q] ][ r+dy[q] ]-dz );

        v.u = x*texCoordScale;
        v.v = y*texCoordScale;

        v.normal[0] = heightAt(i+dx[q]-1,r+dy[q]) - heightAt(i+dx[q]+1,r+dy[q]);
        v.normal[1] = heightAt(i+dx[q],r+dy[q]-1) - heightAt(i+dx[q],r+dy[q]+1);
        v.normal[2] = 1;

        v.normal[0] /= quadSize;
        v.normal[1] /= quadSize;
        double l = sqrt(v.normal[0]*v.normal[0] + v.normal[1]*v.normal[1] + 1);

        for( int t=0; t<3; ++t )
          v.normal[t] /= l;

        land.push_back(v);
        }
  MyGL::VertexDeclaration::Declarator decl;
  decl.add( MyGL::Decl::float3, MyGL::Usage::Position )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord )
      .add( MyGL::Decl::float3, MyGL::Usage::Normal   );

  model.load( vboHolder, iboHolder, land, decl );

  return model;
  }

MyGL::Model<Terrain::WVertex>
      Terrain::waterGeometry( MyGL::VertexBufferHolder & vboHolder,
                              MyGL::IndexBufferHolder  & iboHolder) const {
  MyGL::Model<Terrain::WVertex> model;
  WVertex v;// = {0,0,0, 0,0, {0,0,1}, 1};
  v.h = 0;
  v.dir[0] = 1;
  v.dir[1] = 1;

  std::vector< WVertex > land;
  const int dx[] = {0, 1, 1, 0, 1, 0},
            dy[] = {0, 0, 1, 0, 1, 1};

  const double texCoordScale = 0.1;

  for( int i=0; i+1<heightMap.width(); ++i )
    for( int r=0; r+1<heightMap.height(); ++r )
      if( depthAt(i,r  )>0 || depthAt(i+1,r  )>0 ||
          depthAt(i,r+1)>0 || depthAt(i+1,r+1)>0 )
        for( int q=0; q<6; ++q ){
          int x = (i+dx[q]),
              y = (r+dy[q]);
          v.x = World::coordCast( x*quadSize );
          v.y = World::coordCast( y*quadSize );

          //int dz = waterMap[ i+dx[q] ][ r+dy[q] ];
          v.z = World::coordCast( heightMap[ i+dx[q] ][ r+dy[q] ] )-0.2;

          v.u = x*texCoordScale;
          v.v = y*texCoordScale;

          v.normal[0] =  heightAtNoDepth(i+dx[q]-1,r+dy[q])
                        -heightAtNoDepth(i+dx[q]+1,r+dy[q]);
          v.normal[1] =  heightAtNoDepth(i+dx[q],r+dy[q]-1)
                        -heightAtNoDepth(i+dx[q],r+dy[q]+1);
          v.normal[2] = 1;

          v.normal[0] /= quadSize;
          v.normal[1] /= quadSize;
          double l = sqrt(v.normal[0]*v.normal[0] + v.normal[1]*v.normal[1] + 1);

          for( int t=0; t<3; ++t )
            v.normal[t] /= l;

          v.h = World::coordCast( depthAt(i+dx[q], r+dy[q]) )-0.2;
          v.h = v.h*3;
          v.h = std::min( 1.0f, std::max(v.h, 0.0f) );

          v.dir[0] =  heightAt(i+dx[q],r+dy[q]-1)
                     -heightAt(i+dx[q],r+dy[q]+1);

          v.dir[1] =  heightAt(i+dx[q]-1,r+dy[q])
                     -heightAt(i+dx[q]+1,r+dy[q]);

          { v.dir[0] = 1;//0.5*(x-width()/2);//double(width());
            v.dir[1] = 1;//0.5*(y-height()/2);//double(height());
/*
            if( fabs(v.dir[0]) > fabs(v.dir[1]) ){
              v.dir[0] = sgn(v.dir[0]);
              v.dir[1] = 0;
              } else {
              v.dir[0] = 0;
              v.dir[1] = sgn(v.dir[1]);
              }

            float l = sqrt( v.dir[0]*v.dir[0] + v.dir[1]*v.dir[1] );
            v.dir[0] /= l;
            v.dir[1] /= l;

            float k = clamp( (World::coordCast( depthAt(i+dx[q], r+dy[q]) )-0.2)*0.75,
                             0.0, 1.0 );
            v.dir[0] *= (1-k);
            v.dir[1] *= (1-k);
*/
            /*
            if( x*2>width() )
              v.dir[0] =  1; else
              v.dir[0] = -1;

            if( y*2>height() )
              v.dir[1] =  1; else
              v.dir[1] = -1;
              */
            }
          land.push_back(v);
          }

  MyGL::VertexDeclaration::Declarator decl;
  decl.add( MyGL::Decl::float3, MyGL::Usage::Position )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord )
      .add( MyGL::Decl::float3, MyGL::Usage::Normal   )
      .add( MyGL::Decl::float1, MyGL::Usage::Depth    )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord, 1 );

  model.load( vboHolder, iboHolder, land, decl );

  return model;
  }

int Terrain::width() const {
  return heightMap.width();
  }

int Terrain::height() const {
  return heightMap.height();
  }

double Terrain::viewWidth() const {
  return World::coordCast( quadSize*width() );
  }

double Terrain::viewHeight() const {
  return World::coordCast( quadSize*height() );
  }

void Terrain::brushHeight(int x, int y, double dh, double R ) {
  x/=quadSize;
  y/=quadSize;

  for( int i=0; i<width(); ++i )
    for( int r=0; r<height(); ++r ){
      double factor = std::max(0.0,
                               (R-sqrt((x-i)*(x-i)+(y-r)*(y-r))) )/R;
      factor = 1.0-(1.0-factor)*(1.0-factor);

      waterMap[i][r] -= dh*factor;
      }

  computeEnableMap();
  }

int Terrain::at(int x, int y) const {
  return heightMap[x][y];
  }

int Terrain::heightAt(int x, int y) const {
  x = std::max(0, std::min(x, width()-1) );
  y = std::max(0, std::min(y, height()-1) );

  return heightMap[x][y] - waterMap[x][y];
  }

int Terrain::heightAt( float fx, float fy ) const {
  int x = fx, y = fy;
  float lx = fx-x, ly = fy-y;

  int   h00 = heightAt(x,   y  ),
        h10 = heightAt(x+1, y  ),
        h01 = heightAt(x,   y+1),
        h11 = heightAt(x+1, y+1);

  float mid = h00+lx*(h11-h00);

  if( ly < lx ){
    float m2 = h00 + lx*(h10-h00);
    if( lx>0 )
      return m2 +(ly/lx)*(mid-m2); else
      return m2;
    } else {
    float m2 = h01 + lx*(h11-h01);
    if( lx<1 )
      return mid + ((ly-lx)/(1-lx))*(m2-mid); else
      return m2;
  }
}

bool Terrain::isEnableW(int x, int y) const {
  return isEnable( World::coordCast(x),
                   World::coordCast(y) );
  }

bool Terrain::isEnable( int x, int y ) const {
  if( 0<=x && x<enableMap.width() &&
      0<=y && y<=enableMap.height() )
    return enableMap.at(x,y) && (buildingsMap.at(x,y)==0);

  return 0;
  }

bool Terrain::isEnableForBuilding( int x, int y ) const {
  if( 0<=x && x<enableMap.width() &&
      0<=y && y<=enableMap.height() )
    return enableMap.at(x,y) && (buildingsMap.at(x,y)==0);

  return 0;
  }

void Terrain::resetBusyMap() {
  for( int i=0; i<busyMapsCount; ++i )
    busyMap[i].reset();
  }

int Terrain::busyAt(int ix, int iy, int sz) const {
  int summ = 0;
  int id = std::max(0, std::min(sz-1, busyMapsCount-1) );

  for( int i=0; i<=id; ++i ){
    int n = i+1, x = ix/n, y = iy/n;

    x = std::max(0, std::min(x, busyMap[i].count.width()-1) );
    y = std::max(0, std::min(y, busyMap[i].count.height()-1) );

    summ += busyMap[i].count[x][y];
    }

  return summ;
  }

GameObject *Terrain::unitAt(int ix, int iy) const {
  GameObject * tmp = 0;

  for( int i=0; i<busyMapsCount; ++i ){
    int id = i+1, x = ix/id, y = iy/id;
    if( 0<=x && x<busyMap[i].owner.width() &&
        0<=y && y<busyMap[i].owner.height() )
      tmp = busyMap[i].owner[x][y];

    if( tmp && !tmp->isOnMove() )
      return tmp;

    }

  for( int i=0; i<busyMapsCount; ++i ){
    int id = i+1, x = ix/id, y = iy/id;
    if( 0<=x && x<busyMap[i].owner.width() &&
        0<=y && y<busyMap[i].owner.height() )
      tmp = busyMap[i].owner[x][y];

    if( tmp )
      return tmp;

    }

  return 0;
  }

bool Terrain::isEnableQuad( int x, int y, int size ) const {
  int lx = x-size+1, rx = x+size,
      ly = y-size+1, ry = y+size;

  if( !(0<=lx && rx<=enableMap.width() &&
        0<=ly && ry<=enableMap.height()) )
    return 0;

  for( int i=lx; i<rx; ++i )
    for( int r=ly; r<ry; ++r )
      if( !(enableMap.at(i,r) && (buildingsMap.at(i,r)==0) ) )
        return 0;

  return 1;
  }

void Terrain::incBusyAt(int x, int y, GameObject &owner) {
  if( !owner.isMoviable() || owner.isMineralMove() )
    return;

  int sz = owner.getClass().data.size;

  sz = std::max(1, std::min( sz, busyMapsCount) );
  int id = sz-1;
  x /= sz;
  y /= sz;

  if( 0<=x && x<busyMap[id].count.width() &&
      0<=y && y<busyMap[id].count.height() ){
    ++busyMap[id].count[x][y];

    GameObject *& ptr = busyMap[id].owner[x][y];

    if( ptr==0 ||
        (ptr->isOnMove() && !ptr->isRepositionMove()) ){
      ptr = &owner;
      }
    }
  }

std::pair<int, int> Terrain::nearestEnable( int x, int y, int sz) const {
  return nearestEnable( x,y, x,y, sz );
  }

std::pair<int, int> Terrain::nearestEnable(int x, int y,
                                           int tgX, int tgY, int sz) const {
  int maxR = std::max(enableMap.width(), enableMap.height());

  for( int rad = 0; rad<maxR; ++rad ){
    int lx = x-rad, rx = x+rad,
        ly = y-rad, ry = y+rad;

    //ly = y; //debug trick
    //ry = y;

    lx = clampX(lx);
    rx = clampX(rx);

    ly = clampY(ly);
    ry = clampY(ry);

    int found = -1, fx = 0, fy = 0;

    for( int i=lx; i<=rx; ++i ){
      if( isEnableQuad(i, ly, sz) && busyAt(i,ly, sz)==0 ){
        int d = abs(i-tgX) + abs(ly-tgY);
        if( found<0 || d< found ){
          found = d;
          fx = i;
          fy = ly;
          }
        //return std::make_pair(i, ly);
        }

      if( isEnableQuad(i, ry, sz) && busyAt(i,ry, sz)==0 ){
        int d = abs(i-tgX) + abs(ry-tgY);
        if( found<0 || d< found ){
          found = d;
          fx = i;
          fy = ry;
          }
        }
        //return std::make_pair(i, ry);
      }

    for( int i=ly; i<=ry; ++i ){
      if( isEnableQuad(lx, i, sz) && busyAt(lx, i, sz)==0 ){
        int d = abs(lx-tgX) + abs(i-tgY);
        if( found<0 || d< found ){
          found = d;
          fx = lx;
          fy = i;
          }
        //return std::make_pair(lx, i);
        }

      if( isEnableQuad(rx, i, sz) && busyAt(rx, i, sz)==0 ){
        int d = abs(rx-tgX) + abs(i-tgY);
        if( found<0 || d< found ){
          found = d;
          fx = rx;
          fy = i;
          }
        }

      if( found>=0 ){
        return std::make_pair(fx,fy);
        }
        //return std::make_pair(rx, i);
      }
    }

  return std::make_pair(x,y);
  }

unsigned Terrain::nextGroupMask() const {
  groupMask = (groupMask+1)%65536;
  return groupMask;
  }

void Terrain::editBuildingsMap(int x, int y, int w, int h, int dv) {
  int x1 = x+w, y1 = y+h;

  for( int i=x; i<x1; ++i )
    for( int r=y; r<y1; ++r )
      if( buildingsMap.validate(i,r) )
        buildingsMap[i][r] += dv;
  }

int Terrain::clampX(int x) const {
  if( x<0 )
    return 0;

  if( x>=enableMap.width() )
    return enableMap.width()-1;

  return x;
  }

int Terrain::clampY(int y) const {
  if( y<0 )
    return 0;

  if( y>=enableMap.height() )
    return enableMap.height()-1;

  return y;
  }

void Terrain::computeEnableMap() {
  enableMap.resize( heightMap.width()-1, heightMap.height()-1 );

  for( int i=0; i<enableMap.width(); ++i )
    for( int r=0; r<enableMap.height(); ++r ){
      int h[4] = { heightAt(i,r),
                   heightAt(i+1,r),
                   heightAt(i,r+1),
                   heightAt(i+1,r+1) };

      int det = *std::max_element(h, h+4) - *std::min_element(h,h+4);

      enableMap[i][r] = (det<50);
      }
  }

int Terrain::heightAtNoDepth(int x, int y) const {
  x = std::max(0, std::min(x, width()-1) );
  y = std::max(0, std::min(y, height()-1) );

  return heightMap[x][y];
  }

int Terrain::depthAt(int x, int y) const {
  x = std::max(0, std::min(x, width()-1) );
  y = std::max(0, std::min(y, height()-1) );

  return waterMap[x][y];
  }

void Terrain::BusyMap::resize(int w, int h) {
  count.resize(w,h);
  owner.resize(w,h);
  }

void Terrain::BusyMap::reset() {
  std::fill( count.begin(), count.end(), 0 );
  std::fill( owner.begin(), owner.end(), (GameObject*)0 );
  }

void Terrain::serialize( GameSerializer &s ) {
  int w = heightMap.width()  - 1,
      h = heightMap.height() - 1;

  s + w + h;

  heightMap.resize( w+1, h+1 );
  waterMap .resize( w+1, h+1 );
  buildingsMap.resize(w+1, h+1);

  for( int i=0; i<busyMapsCount; ++i ){
    int n = i+1;
    busyMap[i].resize(w/n,h/n);
    }

  for( int i=0; i<w; ++i )
    for( int r=0; r<h; ++r ){
      s +  heightMap[i][r]
        +  waterMap[i][r]
        +  buildingsMap[i][r];
      }

  resetBusyMap();
  computeEnableMap();
  }
