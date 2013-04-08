#include "terrain.h"

#include <Tempest/Model>

#include "game/world.h"

#include <cmath>
#include "algo/algo.h"
#include "util/gameserializer.h"
#include <memory>
#include <algorithm>

#include "util/tnloptimize.h"

const int Terrain::chunkSize = 32;
//const int Terrain::chunkSize = 16;

Terrain::Terrain( int w, int h,
                  Resource & res,
                  Tempest::VertexBufferHolder & vb,
                  Tempest::IndexBufferHolder  & ib,
                  Scene &s,
                  World &wrld,
                  const PrototypesLoader &pl)
  :scene(s), world(wrld), prototype(pl), vboHolder(vb), iboHolder(ib), res(res) {
  //groupMask = 0;

  land. reserve( w*h*6 );
  minor.reserve( w*h*6 );

  aviableTiles.push_back("land.grass");
  aviableTiles.push_back("land.rock" );
  aviableTiles.push_back("land.sand" );

  for( size_t i=0; i<aviableTiles.size(); ++i ){
    Tempest::Color cl = res.textureAVG(prototype.get(aviableTiles[i]).view[0].name+"/diff");
    aviableColors.push_back( cl );
    }

  tileset.resize( w+1, h+1 );
  tileinf.resize( w+1, h+1 );

  for( int i=0; i<=w; ++i )
    for( int r=0; r<=h; ++r ){
      tileset[i][r].textureID[0] = 0;
      tileset[i][r].textureID[1] = 1;
      }

  heightMap.resize( w+1, h+1 );
  waterMap .resize( w+1, h+1 );
  buildingsMap.resize(w+1, h+1);
  chunks.resize( w/chunkSize+1, h/chunkSize+1 );

  std::fill( heightMap.begin(),    heightMap.end(),     0 );
  std::fill( waterMap.begin(),     waterMap.end(),      0 );
  std::fill( buildingsMap.begin(), buildingsMap.end(),  0 );

  computeEnableMap();
  }

void Terrain::buildGeometry(){
  computePlanes();

  for( int i=0; i<chunks.width(); ++i )
    for( int r=0; r<chunks.height(); ++r ){
      chunks[i][r].needToUpdate = true;
      if( chunks[i][r].needToUpdate )
        chunks[i][r].landView.clear();
      }

  std::vector<size_t> texIDS[2];
  for( int i=0; i<tileset.width(); ++i )
    for( int r=0; r<tileset.height(); ++r ){
      for( int q=0; q<2; ++q ){
        if( std::find( texIDS[q].begin(), texIDS[q].end(),
                       tileset[i][r].textureID[q] )
            == texIDS[q].end() )
          texIDS[q].push_back( tileset[i][r].textureID[q] );
        }
      }

  for( int i=0; i<3; ++i ){
    const std::vector<size_t> & ids = texIDS[ i==2? 0:1 ];
    for( size_t r=0; r<ids.size(); ++r ){
      buildGeometry( vboHolder, iboHolder, i, ids[r] );
      }
    }

  for( int i=0; i<chunks.width(); ++i )
    for( int r=0; r<chunks.height(); ++r )
      chunks[i][r].needToUpdate  = false;
  }

MVertex Terrain::mkVertex(int x, int y, int plane) {
  const double texCoordScale = 0.1;

  Model::Vertex v = {0,0,0, 0,0, {0,0,1}, {1,1,1,1}, {-1,0,0,0} };

  v.x = World::coordCast( x*quadSize );
  v.y = World::coordCast( y*quadSize );

  int dz   = waterMap[ x ][ y ];
  float z  = -(heightMap[ x ][ y ]-dz)/quadSizef;

  v.z = World::coordCast( heightMap[ x ][ y ]-dz );

  if( y+1 < heightMap.height() ){
    v.bnormal[2] = World::coordCast(  heightMap[ x ][ y+1 ]
                                     - waterMap[ x ][ y+1 ] ) - v.z;
    }

  //int plane = tileset[ i+dx[q] ][ r+dy[q] ].plane;

  if( plane==2 ){
    v.u = x*texCoordScale;
    v.v = y*texCoordScale;
    }
  if( plane==1 ){
    v.u = x*texCoordScale;
    v.v = z*texCoordScale;
    }
  if( plane==0 ){
    v.u = z*texCoordScale;
    v.v = y*texCoordScale;
    }

  float *n = tileset[x][y].normal;
  std::copy( n, n+3, v.normal );

  return v;
  }

bool Terrain::isSame(const MVertex &z0, const MVertex &z1) {
  bool next = ( z0.z==z1.z);
  for( int q=0; q<3; ++q ){
    next &= (z0. normal[q]==z1. normal[q]);
    next &= (z0.bnormal[q]==z1.bnormal[q]);
    }

  return next;
  }

void Terrain::buildGeometry( Tempest::VertexBufferHolder & vboHolder,
                             Tempest::IndexBufferHolder  & iboHolder,
                             int plane,
                             size_t texture,
                             int cX, int cY ) {
  TerrainChunk & chunk = chunks[cX][cY];
  if( !chunk.needToUpdate )
    return;

  int lx = (heightMap.width()* cX   )/chunks.width(),
      rx = (heightMap.width()*(cX+1))/chunks.width()+1;

  int ly = (heightMap.height()* cY   )/chunks.height(),
      ry = (heightMap.height()*(cY+1))/chunks.height()+1;

  rx = std::min(rx, heightMap.width() );
  ry = std::min(ry, heightMap.height());

  Model model;
  land.clear();
  minor.clear();


  const int dx[] = {0, 1, 1, 0, 1, 0},
            dy[] = {0, 0, 1, 0, 1, 1};
  /*
  const int dx[] = {0, 1, 1, 0},
            dy[] = {0, 0, 1, 1};*/
  int dcount = 6;

  for( int i=lx; i+1<rx; ++i )
    for( int r=ly; r+1<ry; ++r ){
      int k = 0, tk = 0, tid = (plane==2 ? 0:1);

      for( int q=0; q<dcount; ++q ){
        Tile & t = tileset[i+dx[q]][r+dy[q]];
        if( plane == t.plane )
          ++k;
        if( texture == t.textureID[tid] )
          ++tk;
        }

      tileinf[i][r].land  = false;
      tileinf[i][r].minor = false;

      if( k!=0 && tk!=0 ){
        tileinf[i][r].land = true;
        if( ( k>0 && (k<dcount || tk<dcount) ) && ( tk>0 && (k<dcount || tk<dcount) ) ){
          tileinf[i][r].minor = true;

          for( int q=0; q<dcount; ++q ){
            tileinf[i][r].black[q] =
                ( plane  !=tileset[i+dx[q]][r+dy[q]].plane ||
                  texture!=tileset[i+dx[q]][r+dy[q]].textureID[tid] );
            }
          }

        if( k!=dcount || tk!=dcount ){
          tileinf[i][r].land = false;
          }
        }
      }

  land.clear();
  minor.clear();
/*
  for( int i=lx; i+1<rx; ++i )
    for( int r=ly; r+1<ry; ){
      TileInfo &inf = tileinf[i][r];
      if( inf.land ){
        int r0 = r, dr0 = 0;
        int r1 = r, dr1 = 0;

        MVertex z0 = mkVertex(  i,r,plane);
        MVertex z1 = mkVertex(i+1,r0,plane);

        int oldDr0 = dr0;
        while( r0+1<ry &&
               tileinf[i][r0].land &&
               (i!=lx && tileinf[i-1][r0].land) &&
               isSame( z0, mkVertex(i,  r0+1,plane) )&&
               isSame( z1, mkVertex(i+1,r0+1,plane) ) ){
          ++r0;
          ++dr0;
          }
        r0  = std::max(r+1,r0);
        dr0 = std::max(1,dr0);

        while( r1!=r0 ){
          if( r1<r0 ){
            land.push_back( mkVertex(i+0, r+0, plane) );
            land.push_back( mkVertex(i+1, r+dr1, plane) );

            //MVertex z0 = mkVertex(i  ,r1,plane);
            MVertex z1 = mkVertex(i+1,r1,plane);

            bool v = false;
            while( r1+1<ry &&
                   //r1<r0 &&
                   tileinf[i  ][r1].land &&
                   tileinf[i+1][r1].land &&
                   isSame( z0, mkVertex(i  ,r1+1,plane) ) &&
                   isSame( z1, mkVertex(i+1,r1+1,plane) )
                   ){
              ++r1;
              ++dr1;
              v = true;
              }

            if( !v ){
              ++r1;
              ++dr1;
              }

            oldDr0 = dr0;
            land.push_back( mkVertex(i+1, r+dr1, plane) );
            } else {
            MVertex z0 = mkVertex(i  ,r0,plane);
            MVertex z1 = mkVertex(i+1,r0,plane);

            bool v = false;
            while( r0+1<ry &&
                   tileinf[i][r0].land &&
                   (i!=lx && tileinf[i-1][r0].land) &&
                   isSame( z0, mkVertex(i  ,r0+1,plane) )&&
                   isSame( z1, mkVertex(i+1,r0+1,plane) )   ){
              ++r0;
              ++dr0;
              v = true;
              }

            if( !v ){
              ++r0;
              ++dr0;
              }

            land.push_back( mkVertex(i+0, r+dr0, plane) );
            land.push_back( mkVertex(i+0, r+oldDr0, plane) );
            oldDr0 = dr0;
            land.push_back( mkVertex(i+1, r+std::max(dr0, dr1), plane) );
            }
          }

        land.push_back( mkVertex(i+0, r+dr0, plane) );
        land.push_back( mkVertex(i+0, r+oldDr0, plane) );
        land.push_back( mkVertex(i+1, r+std::max(dr0, dr1), plane) );

        r = std::max(r1, r0);
        } else {
        ++r;
        }

      }*/

  for( int i=lx; i+1<rx; ++i )
    for( int r=ly; r+1<ry; ++r ){
      TileInfo &inf = tileinf[i][r];

      if( inf.land ){
        for( int q=0; q<dcount; ++q ){
          MVertex v = mkVertex(i+dx[q], r+dy[q], plane);
          land.push_back( v );
          }
        }
      }

  for( int i=lx; i+1<rx; ++i )
    for( int r=ly; r+1<ry; ++r ){
      TileInfo &inf = tileinf[i][r];

      if( inf.minor ){
        for( int q=0; q<dcount; ++q ){
          MVertex v = mkVertex(i+dx[q], r+dy[q], plane);
          if( inf.black[q] )
            std::fill(v.color+0, v.color+4, 0); else
            std::fill(v.color+0, v.color+4, 1);
          minor.push_back( v );
          }
        }
      }

  std::vector<uint16_t> index;
  if( land.size() ){
    TnlOptimize::index( land, index );
    model.load( vboHolder, iboHolder,
                land, index, MVertex::decl() );

    TerrainChunk::View view;
    ProtoObject obj = prototype.get( aviableTiles[texture] );
    for( size_t i=0; i<obj.view.size(); ++i ){
      for( size_t r=0; r<obj.view[i].materials.size(); ++r ){
        if( obj.view[i].materials[r]=="phong" )
          obj.view[i].materials[r] = "terrain.main";
        }
      }

    view.view.reset( new GameObjectView( scene,
                                         world,
                                         obj,
                                         prototype) );
    view.view->loadView( model, obj.view[0] );

    chunk.landView.push_back( view );
    }

  if( minor.size() ){
    TnlOptimize::index( minor, index );
    model.load( vboHolder, iboHolder, minor, index, MVertex::decl() );

    TerrainChunk::View view;
    ProtoObject obj = prototype.get( aviableTiles[texture] );
    for( size_t i=0; i<obj.view.size(); ++i ){
      for( size_t r=0; r<obj.view[i].materials.size(); ++r ){
        if( obj.view[i].materials[r]=="phong" )
          obj.view[i].materials[r] = "terrain.minor";
        }
      }

    view.view.reset( new GameObjectView( scene,
                                         world,
                                         obj,
                                         prototype) );
    view.view->loadView( model, obj.view[0] );

    chunk.landView.push_back( view );
    }

  if( plane==2 ){
    chunk.waterView.view.reset( new GameObjectView( scene,
                                                    world,
                                                    prototype.get( "water" ),
                                                    prototype) );
    chunk.waterView.view->loadView( waterGeometry(cX, cY) );

    chunk.fogView.view.reset( new GameObjectView( scene,
                                                  world,
                                                  prototype.get( "water" ),
                                                  prototype) );
    ProtoObject::View vf;
    vf.materials.clear();
    vf.materials.push_back("fog_of_war");

    chunk.fogView.view->loadView( fogGeometry(cX, cY), vf );
    }
  }

void Terrain::buildGeometry( Tempest::VertexBufferHolder & vboHolder,
                             Tempest::IndexBufferHolder  & iboHolder,
                             int plane,
                             size_t texture  ) {
  for( int i=0; i<chunks.width(); ++i )
    for( int r=0; r<chunks.height(); ++r ){
      buildGeometry( vboHolder, iboHolder, plane, texture,
                     i, r );
      }
  }

void Terrain::computePlanes() {
  Model::Vertex v = {0,0,0, 0,0, {0,0,1}, {1,1,1,1}, {-1,0,0,0} };

  for( int i=0; i+1<heightMap.width(); ++i )
    for( int r=0; r+1<heightMap.height(); ++r ){
      v.normal[0] = heightAt(i-1,r) - heightAt(i+1,r);
      v.normal[1] = heightAt(i,r-1) - heightAt(i,r+1);
      v.normal[2] = 1;

      v.normal[0] /= quadSize;
      v.normal[1] /= quadSize;
      double l = sqrt(v.normal[0]*v.normal[0] + v.normal[1]*v.normal[1] + 1);

      for( int t=0; t<3; ++t )
        v.normal[t] /= l;

      float n[3] = {};

      std::copy( v.normal, v.normal+3, n );
      for( int q=0; q<3; ++q )
        n[q] = fabs(n[q]);

      int nplane = 2;
      if( n[1]>=n[2] && n[1]>=n[0] )
        nplane = 1;
      if( n[0]>=n[1] && n[0]>=n[2] )
        nplane = 0;

      tileset[i][r].plane = nplane;

      int id = tileset[i][r].textureID[0];
      tileset[i][r].color = aviableColors[id];
      std::copy( v.normal, v.normal+3, tileset[i][r].normal );
      }

  for( int i=1; i+1<heightMap.width(); ++i )
    for( int r=1; r+1<heightMap.height(); ++r ){
      if( tileset[i][r].normal[2]>0.5 ){
        tileset[i-1][r].plane = 2;
        tileset[i][r-1].plane = 2;

        tileset[i+1][r].plane = 2;
        tileset[i][r+1].plane = 2;
        }
      }
  }

Tempest::Model<WaterVertex>
      Terrain::waterGeometry(int cX, int cY ) const {
  Tempest::Model<WaterVertex> model;
  WaterVertex v;// = {0,0,0, 0,0, {0,0,1}, 1};
  v.h = 0;
  v.dir[0] = 1;
  v.dir[1] = 1;

  std::fill( v.color, v.color+4, 1 );

  std::vector< WaterVertex > land;
  const int dx[] = {0, 1, 1, 0, 1, 0},
            dy[] = {0, 0, 1, 0, 1, 1};

  const double texCoordScale = 0.1/2.0;

  int lx = (heightMap.width()* cX   )/chunks.width(),
      rx = (heightMap.width()*(cX+1))/chunks.width()+1;

  int ly = (heightMap.height()* cY   )/chunks.height(),
      ry = (heightMap.height()*(cY+1))/chunks.height()+1;

  rx = std::min(rx, heightMap.width() );
  ry = std::min(ry, heightMap.height());

  for( int i=lx; i+1<rx; ++i )
    for( int r=ly; r+1<ry; ++r )
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
            }
          land.push_back(v);
          }

  Tempest::VertexDeclaration::Declarator decl = MVertex::decl();
  decl.add( Tempest::Decl::float1, Tempest::Usage::Depth    )
      .add( Tempest::Decl::float2, Tempest::Usage::TexCoord, 1 );

  model.load( vboHolder, iboHolder, land, decl );

  return model;
  }

Model Terrain::fogGeometry(int cX, int cY ) const {
  Model model;
  MVertex v;// = {0,0,0, 0,0, {0,0,1}, 1};

  std::fill( v.color, v.color+4, 1 );

  std::vector< MVertex > land;
  const int dx[] = {0, 1, 1, 0, 1, 0},
            dy[] = {0, 0, 1, 0, 1, 1};

  int lx = (heightMap.width()* cX   )/chunks.width(),
      rx = (heightMap.width()*(cX+1))/chunks.width()+1;

  int ly = (heightMap.height()* cY   )/chunks.height(),
      ry = (heightMap.height()*(cY+1))/chunks.height()+1;

  rx = std::min(rx, heightMap.width() );
  ry = std::min(ry, heightMap.height());

  for( int i=lx; i+1<rx; ++i )
    for( int r=ly; r+1<ry; ++r )
        for( int q=0; q<6; ++q ){
          int x = (i+dx[q]),
              y = (r+dy[q]);
          v.x = World::coordCast( x*quadSize );
          v.y = World::coordCast( y*quadSize );

          int dz = waterMap[ i+dx[q] ][ r+dy[q] ];
          if( dz<0 )
            v.z = World::coordCast( heightMap[ i+dx[q] ][ r+dy[q] ]-dz ); else
            v.z = World::coordCast( heightMap[ i+dx[q] ][ r+dy[q] ] );

          v.u = x/double( heightMap.width() );
          v.v = y/double( heightMap.height() );

          v.normal[0] = 0;
          v.normal[1] = 0;
          v.normal[2] = 1;

          land.push_back(v);
          }

  Tempest::VertexDeclaration::Declarator decl = MVertex::decl();
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

void Terrain::brushHeight( int x, int y,
                           const Terrain::EditMode &m,
                           bool alternative ) {
  x/=quadSize;
  y/=quadSize;

  double R = m.R, dh = 800;

  int iR = int(R+2);
  int lx = std::max(0, x-iR), rx = std::min(width(),  x+iR);
  int ly = std::max(0, y-iR), ry = std::min(height(), y+iR);

  if( alternative )
    dh = -dh;

  if( m.map==EditMode::Align ){
    x = std::max(0, std::min(x, heightMap.width() -1) );
    y = std::max(0, std::min(y, heightMap.height()-1) );

    int h  = heightMap[x][y];
    int hW =  waterMap[x][y];

    for( int i=lx; i<rx; ++i )
      for( int r=ly; r<ry; ++r ){
        double factor = std::max(0.0,
                                 (R-sqrt( (double)(x-i)*(x-i)+(y-r)*(y-r))) )/R;
        factor = (1.0-factor)*(1.0-factor);
        factor = 1.0 - std::max(factor-0.8, 0.0)/0.2;

        for( int dx = 0; dx<2; ++dx)
          for( int dy = 0; dy<2; ++dy )
            if( chunks.validate(i/chunkSize+dx, r/chunkSize+dy) )
              chunks[i/chunkSize+dx][r/chunkSize+dy].needToUpdate = true;

        heightMap[i][r] += (h -heightMap[i][r])*factor;
        waterMap [i][r] += (hW- waterMap[i][r])*factor;
        }
    } else
  if( m.wmap!= EditMode::None || m.map!=EditMode::None ){
    for( int i=lx; i<rx; ++i )
      for( int r=ly; r<ry; ++r ){
        double factor = std::max(0.0,
                                 (R-sqrt((double)(x-i)*(x-i)+(y-r)*(y-r))) )/R;
        factor = 1.0-(1.0-factor)*(1.0-factor);

        for( int dx = 0; dx<2; ++dx)
          for( int dy = 0; dy<2; ++dy )
            if( chunks.validate(i/chunkSize+dx, r/chunkSize+dy) )
              chunks[i/chunkSize+dx][r/chunkSize+dy].needToUpdate = true;

        if( m.wmap ){
          heightMap[i][r] += dh*factor;
          } else {
          waterMap[i][r] -= dh*factor;
          }
        }
    }

  int texID = m.isSecondaryTexturing ? 1:0;
  if( m.texture.size() ){
    int id = std::find( aviableTiles.begin(), aviableTiles.end(), m.texture )
              - aviableTiles.begin();
    if( size_t(id)==aviableTiles.size() ){
      aviableTiles.push_back( m.texture );
      Tempest::Color cl = res.textureAVG(prototype.get(m.texture).view[0].name+"/diff");
      aviableColors.push_back( cl );
      }

    for( int i=lx; i<rx; ++i )
      for( int r=ly; r<ry; ++r ){
        double factor = std::max(0.0,
                                 (R-sqrt((double)(x-i)*(x-i)+(y-r)*(y-r))) )/R;
        factor = 1.0-(1.0-factor)*(1.0-factor);

        if( factor>0 ){
          tileset[i][r].textureID[texID] = id;
          }

        for( int dx = 0; dx<2; ++dx)
          for( int dy = 0; dy<2; ++dy )
            if( chunks.validate(i/chunkSize+dx, r/chunkSize+dy) )
              chunks[i/chunkSize+dx][r/chunkSize+dy].needToUpdate = true;
        }
    }

  computeEnableMap();
  }

int Terrain::at(int x, int y) const {
  x = std::max(0, std::min(x, width()-1) );
  y = std::max(0, std::min(y, height()-1) );

  return heightMap[x][y];
  }

int Terrain::atF(float fx, float fy) const {
  int x = fx, y = fy;
  float lx = fx-x, ly = fy-y;

  int   h00 = at(x,   y  ),
        h10 = at(x+1, y  ),
        h01 = at(x,   y+1),
        h11 = at(x+1, y+1);

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

void Terrain::normalAt(int x, int y, float *out) {
  x = std::max(0, std::min(x, width()-1) );
  y = std::max(0, std::min(y, height()-1) );

  std::copy( tileset[x][y].normal, tileset[x][y].normal+3, out );
  }

Tempest::Color Terrain::colorAt(int x, int y) {
  x = std::max(0, std::min(x, width()-1) );
  y = std::max(0, std::min(y, height()-1) );

  return tileset[x][y].color;
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
      if( isEnableQuad(i, ly, sz) ){
        int d = abs(i-tgX) + abs(ly-tgY);
        if( found<0 || d< found ){
          found = d;
          fx = i;
          fy = ly;
          }
        //return std::make_pair(i, ly);
        }

      if( isEnableQuad(i, ry, sz) ){
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
      if( isEnableQuad(lx, i, sz) ){
        int d = abs(lx-tgX) + abs(i-tgY);
        if( found<0 || d< found ){
          found = d;
          fx = lx;
          fy = i;
          }
        //return std::make_pair(lx, i);
        }

      if( isEnableQuad(rx, i, sz) ){
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

      enableMap[i][r] = (det<4*75);
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

void Terrain::serialize( GameSerializer &s ) {
  int w = heightMap.width()  - 1,
      h = heightMap.height() - 1;

  s + w + h;

  heightMap.resize( w+1, h+1 );
  waterMap .resize( w+1, h+1 );
  buildingsMap.resize(w+1, h+1);
  tileset.resize( w+1, h+1 );
  tileinf.resize( w+1, h+1 );
  chunks.resize( w/chunkSize+1, h/chunkSize+1 );

  if( s.isReader() )
    for( int i=0; i<chunks.width(); ++i )
      for( int r=0; r<chunks.height(); ++r ){
        chunks[i][r] = TerrainChunk();
        }

  for( int i=0; i<w; ++i )
    for( int r=0; r<h; ++r ){
      s +  heightMap[i][r]
        +  waterMap[i][r]
        +  buildingsMap[i][r];
      }

  if( s.version()>0 ){
    for( int i=w; i<=w; ++i )
      for( int r=h; r<=h; ++r ){
        s +  heightMap[i][r]
          +  waterMap[i][r]
          +  buildingsMap[i][r];
        }

    size_t ts = aviableTiles.size();
    s + ts;
    aviableTiles.resize(ts);

    for( size_t i=0; i<aviableTiles.size(); ++i )
      s + aviableTiles[i];

    aviableColors.clear();
    for( size_t i=0; i<aviableTiles.size(); ++i ){
      Tempest::Color cl = res.textureAVG(prototype.get(aviableTiles[i]).view[0].name+"/diff");
      aviableColors.push_back( cl );
      }

    for( int i=0; i<=w; ++i )
      for( int r=0; r<=h; ++r ){
        Tile & t = tileset[i][r];
        s   + t.plane
            + t.textureID[0]
            + t.textureID[1];
        }
    }

  if( s.version()<5 ){
    for( int i=0; i<=w; ++i )
      for( int r=0; r<=h; ++r ){
        heightMap[i][r] *= 4;
        waterMap [i][r] *= 4;
        }
    }

  if( s.isReader() ){
    std::fill( buildingsMap.begin(), buildingsMap.end(),  0 );
    }

  computeEnableMap();
  }

void Terrain::updatePolish() {
  for( int i=0; i<chunks.width(); ++i )
    for( int r=0; r<chunks.height(); ++r )
      chunks[i][r].update( res );
  }

void Terrain::loadFromPixmap(const Tempest::Pixmap &p) {
  for( int i=0; i<p.width(); ++i )
    for( int r=0; r<p.height(); ++r ){
      Tempest::Pixmap::Pixel px = p.at(i,r);
      //if( px.b>0 )
        //px.b = 32;

      heightMap[i][r] = 64*(32);
      waterMap[i][r]  = 64*(-px.r+px.b);
      //heightMap[i][r] -= 64*(px.b);
      }

  computeEnableMap();
  }


Terrain::EditMode::EditMode() {
  map  = None;
  wmap = None;

  R = 5;
  isEnable = false;
  isSecondaryTexturing = false;
  }
