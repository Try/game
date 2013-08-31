#include "terrain.h"

#include <Tempest/Model>

#include "game/world.h"

#include <cmath>
#include "algo/algo.h"
#include "util/gameserializer.h"
#include <memory>
#include <algorithm>

#include "util/tnloptimize.h"

//const int Terrain::chunkSize = 32;
const int Terrain::chunkSize = 16;

const int   Terrain::quadSize  = 600;
const float Terrain::quadSizef = 600;

Terrain::Terrain( int w, int h,
                  Resource & res,
                  Tempest::VertexBufferHolder & vb,
                  Tempest::IndexBufferHolder  & ib,
                  Scene &s,
                  World &wrld,
                  const PrototypesLoader &pl)
  :scene(s), world(wrld), prototype(pl), vboHolder(vb), iboHolder(ib), res(res) {
  //groupMask = 0;

  int sz = chunkSize*chunkSize*4;
  land. reserve( sz*4 );
  minor.reserve( sz*4 );
  ibo.  reserve(sz*6);

  quads. reserve(sz);
  squads.reserve(sz);

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
  //return;

  computePlanes();

  for( int i=0; i<chunks.width(); ++i )
    for( int r=0; r<chunks.height(); ++r ){
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
      buildGeometry( vboHolder, iboHolder, i, ids[r], (r==0 && i==0) );
      }
    }

  for( int i=0; i<chunks.width(); ++i )
    for( int r=0; r<chunks.height(); ++r )
      chunks[i][r].needToUpdate  = false;
  }

void Terrain::mkTexCoord( float &u, float &v, float x, float y ){
  static const double texCoordScale = World::coordCastD(0.1)/quadSizef;

  u = x*texCoordScale;
  v = y*texCoordScale;
  }

MVertex Terrain::mkVertex(int ix, int iy, int plane) {
  float bnormal[3] = {-1,0,0};
  float x, y, z, u, v;

  int dz    = waterMap[ ix ][ iy ];

  x = World::coordCast( ix*quadSize );
  y = World::coordCast( iy*quadSize );
  z = World::coordCast( heightMap[ ix ][ iy ]-dz );

  if( iy+1 < heightMap.height() ){
    bnormal[2] = World::coordCast(  heightMap[ ix ][ iy+1 ]
                                   - waterMap[ ix ][ iy+1 ] ) - z;
    }

  //int plane = tileset[ i+dx[q] ][ r+dy[q] ].plane;

  if( plane==2 ){
    mkTexCoord(u,v, x,y);
    }
  if( plane==1 ){
    mkTexCoord(u,v, x,z);
    }
  if( plane==0 ){
    mkTexCoord(u,v, z,y);
    }

  float *n = tileset[ix][iy].normal;

  MVertex vx;
  vx.x = x;
  vx.y = y;
  vx.z = z;

  vx.u = u;
  vx.v = v;

  vx.nx = n[0];
  vx.ny = n[1];
  vx.nz = n[2];

  vx.bx = bnormal[0];
  vx.by = bnormal[1];
  vx.bz = bnormal[2];

  return vx;
  }

void Terrain::buildShadowVBO( int lx, int rx, int ly, int ry,
                              std::vector<MVertex>& land,
                              std::vector<uint16_t> & ibo ) {
  land.clear();
  quads.clear();
  ibo.clear();

  static const int dx[] = {0, 1, 1, 0},
                   dy[] = {0, 0, 1, 1};
  //static int dcount = 4;
  int di[] = {0,1,2, 0,2,3};

  int w = ry-ly;
  for( int i=lx; i<rx; ++i )
    for( int r=ly; r<ry; ++r ){
      land.push_back( mkVertex(i, r, 0) );
      }

  for( int i=lx; i+1<rx; ++i )
    for( int r=ly; r+1<ry; ++r ){
      for( int q=0; q<6; ++q ){
        int ti = i+dx[di[q]]-lx,
            tr = r+dy[di[q]]-ly;
        ibo.push_back( ti*w+tr );
        }
      }
  }

void Terrain::buildVBO( int lx, int rx, int ly, int ry,
                        std::vector<MVertex>& land,
                        std::vector<uint16_t> & ibo,
                        bool isLand,
                        int plane ) {
  land.clear();
  quads.clear();
  ibo.clear();

  static const int dx[] = {0, 1, 1, 0},
                   dy[] = {0, 0, 1, 1};
  static int dcount = 4;

  for( int i=lx; i+1<rx; ++i )
    for( int r=ly; r+1<ry; ++r ){
      TileInfo &inf = tileinf[i][r];

      if( (inf.land && isLand) || (inf.minor && !isLand) ){
        quads.push_back( Tempest::Point(i,r) );
        }
      }

  squads.clear();
  Tempest::Point p;
  while( quads.size() ){
    Tempest::Point px = quads[0];
    int dp = abs(px.x-p.x) + abs(px.y-p.y), id = 0;

    for( size_t i=1; i<quads.size(); ++i ){
      int ndp = abs(quads[i].x-p.x) + abs(quads[i].y-p.y);
      if( ndp<dp ){
        id = i;
        px = quads[i];
        dp = ndp;
        }
      }

    squads.push_back(px);
    quads[id] = quads.back();
    quads.pop_back();
    }

  int di[] = {0,1,2, 0,2,3};

  ibo.clear();
  for( size_t id=0; id<squads.size(); ++id ){
    int i= squads[id].x, r = squads[id].y;

    TileInfo &inf = tileinf[i][r];

    size_t r0 = std::max<size_t>( 0, land.size()-1024 );

    MVertex vx[dcount];
    for( int q=0; q<dcount; ++q ){
      vx[q] = mkVertex(i+dx[q], r+dy[q], plane);
      if( !isLand )
        vx[q].h = inf.black[q]?0:1;
      //land.push_back( v );
      }

    for( int r=0; r<6; ++r ){
      const MVertex& v = vx[ di[r] ];
      size_t id = size_t(-1);
      for( size_t q=r0; q<land.size(); ++q ){
        if( land[q]==v ){
          id = q;
          }
        }

      if( id==size_t(-1) ){
        ibo.push_back( land.size() );
        land.push_back(v);
        } else {
        ibo.push_back(id);
        }
      //ibo.push_back(r1+di[r]);
      }
    }

  }

void Terrain::buildGeometry( Tempest::VertexBufferHolder & vboHolder,
                             Tempest::IndexBufferHolder  & iboHolder,
                             int plane,
                             size_t texture,
                             int cX, int cY,
                             bool firstPass ) {
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

  /*
  const int dx[] = {0, 1, 1, 0, 1, 0},
            dy[] = {0, 0, 1, 0, 1, 1};
  */

  const int dx[] = {0, 1, 1, 0},
            dy[] = {0, 0, 1, 1};
  int dcount = 4;

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

  buildVBO(lx, rx, ly, ry, land,  ibo, true, plane );

  if( land.size() ){
    model.load( vboHolder, iboHolder, land, ibo, MVertex::decl() );
    //model.load( vboHolder, iboHolder, land,  MVertex::decl() );

    TerrainChunk::View view;
    ProtoObject obj = prototype.get( aviableTiles[texture] );
    for( size_t i=0; i<obj.view.size(); ++i ){
      for( size_t r=0; r<obj.view[i].materials.size(); ++r ){
        if( obj.view[i].materials[r]=="phong" )
          obj.view[i].materials[r] = "terrain.main";
        }

      remove(obj.view[i].materials, "shadow_cast");
      }

    view.view.reset( new GameObjectView( scene,
                                         world,
                                         obj,
                                         prototype) );
    view.view->loadView( model, obj.view[0] );

    chunk.landView.push_back( view );
    }

  //return;

  buildVBO(lx, rx, ly, ry, minor, ibo, false, plane );
  if( minor.size() ){
    model.load( vboHolder, iboHolder, minor, ibo, MVertex::decl() );

    TerrainChunk::View view;
    ProtoObject obj = prototype.get( aviableTiles[texture] );
    for( size_t i=0; i<obj.view.size(); ++i ){
      /*
      for( size_t r=0; r<obj.view[i].materials.size(); ++r ){
        if( obj.view[i].materials[r]=="phong" )
          obj.view[i].materials[r] = "terrain.minor";
        }*/
      remove(obj.view[i].materials, "shadow_cast");
      }

    view.view.reset( new GameObjectView( scene,
                                         world,
                                         obj,
                                         prototype) );
    view.view->loadView( model, obj.view[0] );

    chunk.landView.push_back( view );
    }

  if( firstPass ){
    chunk.waterView.view.reset( new GameObjectView( scene,
                                                    world,
                                                    prototype.get( "water" ),
                                                    prototype) );
    chunk.waterView.view->loadView( waterGeometry(cX, cY) );
    return;//TODO

    chunk.fogView.view.reset( new GameObjectView( scene,
                                                  world,
                                                  prototype.get( "water" ),
                                                  prototype) );
    ProtoObject::View vf;
    vf.materials.clear();
    vf.materials.push_back("fog_of_war");

    chunk.fogView.view->loadView( fogGeometry(cX, cY), vf );

    buildShadowVBO(lx, rx, ly, ry, land, ibo );
    if( land.size() ){
      model.load( vboHolder, iboHolder, land, ibo, MVertex::decl() );

      TerrainChunk::View view;

      ProtoObject obj;
      obj.view.resize(1);
      obj.view[0].materials.clear();
      obj.view[0].materials.push_back("shadow_cast");

      view.view.reset( new GameObjectView( scene,
                                           world,
                                           obj,
                                           prototype) );
      view.view->loadView( model, obj.view[0] );

      chunk.landView.push_back( view );
      }
    }
  }

void Terrain::buildGeometry( Tempest::VertexBufferHolder & vboHolder,
                             Tempest::IndexBufferHolder  & iboHolder,
                             int plane,
                             size_t texture,
                             bool firstPass ) {
  for( int i=0; i<chunks.width(); ++i )
    for( int r=0; r<chunks.height(); ++r ){
      buildGeometry( vboHolder, iboHolder, plane, texture,
                     i, r,
                     firstPass );
      }
  }

void Terrain::computePlanes() {
  //Model::Vertex v = {0,0,0,0, {0,0,1}, {1,1,1,1}, {-1,0,0}, 0 };

  for( int i=0; i+1<heightMap.width(); ++i )
    for( int r=0; r+1<heightMap.height(); ++r ){
      float normal[3];
      normal[0] = (heightAt(i-1,r) - heightAt(i+1,r))/quadSize;
      normal[1] = (heightAt(i,r-1) - heightAt(i,r+1))/quadSize;
      normal[2] = 1;

      double l = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + 1);

      for( int t=0; t<3; ++t )
        normal[t] /= l;

      float n[3] = {};

      std::copy( normal, normal+3, n );
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
      std::copy( normal, normal+3, tileset[i][r].normal );
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

  //std::fill( v.color, v.color+4, 1 );

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

          v.u = 2*x*texCoordScale;
          v.v = 2*y*texCoordScale;

          float normal[3];
          normal[0] = ( heightAtNoDepth(i+dx[q]-1,r+dy[q])
                       -heightAtNoDepth(i+dx[q]+1,r+dy[q]));
          normal[1] = (heightAtNoDepth(i+dx[q],r+dy[q]-1)
                       -heightAtNoDepth(i+dx[q],r+dy[q]+1));
          normal[2] = 1;

          normal[0] /= quadSize;
          normal[1] /= quadSize;
          double l = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + 1);

          v.nx = normal[0]/l;
          v.ny = normal[1]/l;
          v.nz = normal[2]/l;

          float  bnormal[3] = {-1, 0, 0};
          bnormal[2] = heightAtNoDepth(i+dx[q],r+dy[q]+1) -
                       heightAtNoDepth(i+dx[q],r+dy[q]);
          l = sqrt( bnormal[0]*bnormal[0] +
                    bnormal[1]*bnormal[1] +
                    bnormal[2]*bnormal[2]) ;

          v.bx = bnormal[0]/l;
          v.by = bnormal[1]/l;
          v.bz = bnormal[2]/l;

          v.h = World::coordCast( depthAt(i+dx[q], r+dy[q]) )-0.2;
          v.h = v.h*3;
          v.h = std::min( 1.0f, std::max( float(v.h), 0.0f) );

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
  decl//.add( Tempest::Decl::float1, Tempest::Usage::Depth    )
      .add( Tempest::Decl::half2, Tempest::Usage::TexCoord, 1 );

  std::vector<uint16_t> index;
  TnlOptimize::index( land, index );
  model.load( vboHolder, iboHolder, land, index, decl );

  return model;
  }

Model Terrain::fogGeometry(int cX, int cY ) const {
  Model model;
  MVertex v;// = {0,0,0, 0,0, {0,0,1}, 1};

  //std::fill( v.color, v.color+4, 1 );

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

          v.nx = 0;
          v.ny = 0;
          v.nz = 1;

          std::fill( v.color, v.color+4, 1);

          land.push_back(v);
          }

  Tempest::VertexDeclaration::Declarator decl = MVertex::decl();
  model.load( vboHolder, iboHolder, land, decl );

  return model;
  }

const ProtoObject::View *Terrain::viewAt(float fx, float fy) {
  int x = World::coordCastD(fx)/quadSizef;
  int y = World::coordCastD(fy)/quadSizef;

  x = std::max(0, std::min(x, width()-1) );
  y = std::max(0, std::min(y, height()-1) );

  int v = tileset[x][y].textureID[0];

  if( int(aviableDecalViews.size())<=v ){
    aviableDecalViews.resize(v+1);
    }

  if( !aviableDecalViews[v] ){
    const ProtoObject::View &vx = prototype.get( aviableTiles[v] ).view[0];
    aviableDecalViews[v].reset( new ProtoObject::View(vx) );
    remove(aviableDecalViews[v]->materials, "shadow_cast");
    }

  return aviableDecalViews[v].get();
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

  bool updateEnable = false;
  double R = m.R, dh = 800;

  int iR = int(R+2);
  int lx = std::max(0, x-iR), rx = std::min(width(),  x+iR);
  int ly = std::max(0, y-iR), ry = std::min(height(), y+iR);

  if( alternative )
    dh = -dh;

  if( m.map==EditMode::Smooth ){
    updateEnable = true;

    int maxX = lx,
        maxY = ly,
        minX = lx,
        minY = ly,
        maxH = waterMap[lx][ly],
        minH = maxH;

    int yp[] = {ly, ry};
    int xp[] = {lx, rx};

    for( int p=0; p<2; ++p ){
      for( int i=lx; i<rx; ++i ){
        int h0 = waterMap[i][yp[p]];
        if( h0 > maxH ){
          maxH = h0;
          maxX = i;
          maxY = yp[p];
          }
        if( h0 < minH ){
          minH = h0;
          minX = i;
          minY = yp[p];
          }
        }
      for( int i=ly; i<ry; ++i ){
        int h0 = waterMap[xp[p]][i];
        if( h0 > maxH ){
          maxH = h0;
          maxY = i;
          maxX = xp[p];
          }
        if( h0 < minH ){
          minH = h0;
          minY = i;
          minX = xp[p];
          }
        }
      }

    for( int i=lx; i<rx; ++i )
      for( int r=ly; r<ry; ++r ){

        for( int dx = 0; dx<2; ++dx)
          for( int dy = 0; dy<2; ++dy )
            if( chunks.validate(i/chunkSize+dx, r/chunkSize+dy) )
              chunks[i/chunkSize+dx][r/chunkSize+dy].needToUpdate = true;

        float ax = maxX - minX,
              ay = maxY - minY,
              la = sqrt(ax*ax+ay*ay),
              bx = i-minX,
              by = r-minY;

        float a = (ax*bx+ay*by)/(la*la);
        a = std::min(1.0f, std::max(a, 0.0f));

        waterMap[i][r] = maxH*a + minH*(1.0f-a);
        }
    } else
  if( m.map==EditMode::Align ){
    updateEnable = true;

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
    updateEnable = true;

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

  if( updateEnable ){
    int dw = enableMapUpdateRect.x,
        dh = enableMapUpdateRect.y;
    enableMapUpdateRect.x = std::min(lx, enableMapUpdateRect.x);
    enableMapUpdateRect.y = std::min(ly, enableMapUpdateRect.y);

    enableMapUpdateRect.w += dw - enableMapUpdateRect.x;
    enableMapUpdateRect.h += dh - enableMapUpdateRect.y;

    enableMapUpdateRect.w = std::max(rx-lx, enableMapUpdateRect.w);
    enableMapUpdateRect.h = std::max(ry-ly, enableMapUpdateRect.h);

    computeEnableMap();
    }
  onTerrainChanged();
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

  onTerrainChanged();
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

  if( enableMapUpdateRect.isEmpty() )
    enableMapUpdateRect = Tempest::Rect(0,0,enableMap.width(), enableMap.height());

  for( int i=enableMapUpdateRect.x; i<enableMapUpdateRect.x+enableMapUpdateRect.w; ++i )
    for( int r=enableMapUpdateRect.y; r<enableMapUpdateRect.y+enableMapUpdateRect.h; ++r ){
      int h[4] = { heightAt(i,r),
                   heightAt(i+1,r),
                   heightAt(i,r+1),
                   heightAt(i+1,r+1) };

      int det = *std::max_element(h, h+4) - *std::min_element(h,h+4);

      enableMap[i][r] = (det<4*75);
      }

  enableMapUpdateRect = Tempest::Rect();
  }

int Terrain::heightAtNoDepth(int x, int y) const {
  x = std::max(0, std::min(x, width()-1) );
  y = std::max(0, std::min(y, height()-1) );

  return heightMap[x][y];
  }
/*
const array2d<int> &Terrain::wayCorrMap() const {
  if( needToUpdateWCMap )
    updateWCMap();
  return wcorrMap;
  }
*/
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

  if( s.isReader() )
    onTerrainChanged();

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

  for( int i=0; i<chunks.width(); ++i )
    for( int r=0; r<chunks.height(); ++r )
      chunks[i][r].needToUpdate = 1;

  buildGeometry();
  }


Terrain::EditMode::EditMode() {
  map  = None;
  wmap = None;

  R = 5;
  isEnable = false;
  isSecondaryTexturing = false;
  }
