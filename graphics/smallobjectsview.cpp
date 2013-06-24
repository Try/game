#include "smallobjectsview.h"

#include "landscape/terrain.h"
#include "game.h"
#include "algo/algo.h"

#include <cmath>

SmallGraphicsObject::SmallGraphicsObject( Scene &s,
                                          Game &g,
                                          Terrain &t,
                                          const ProtoObject::View &view )
  :view(view), t(&t), scene(s), game(g) {
  mx = 0;
  my = 0;
  mz = 0;

  sx = 0;
  sy = 0;
  sz = 0;

  model = 0;

  glocation    = -1;
  visible      = true;
  needToUpdate = false;

  chunkBase().polishObj.push_back( this );
  chunk().needToUpdate = true;
  }

SmallGraphicsObject::~SmallGraphicsObject() {
  remove( chunkBase().polishObj, this );
  chunk().needToUpdate = true;
  }

void SmallGraphicsObject::setModel( const Model &m, const std::string & key ) {
  bds = m.bounds();

  model = &game.resources().rawModel( key );
  needToUpdate = true;
  chunk().needToUpdate = true;
  }

void SmallGraphicsObject::setPosition(float ix, float iy, float iz) {
  if( mx==ix && my==iy && mz==iz )
    return;

  bool rm = ( &chunkBase(mx,my)!=&chunkBase(ix,iy) ||
               fabs( chunk().zView - mz )<0.001 );

  if( rm && visible )
    remove( chunkBase().polishObj, this );

  TerrainChunk::PolishView *v1 = &chunk();
  v1->needToUpdate |= rm;

  mx = ix;
  my = iy;
  mz = iz;

  if( rm && visible  )
    chunkBase().polishObj.push_back( this );

  TerrainChunk::PolishView *v2 = &chunk();
  v2->needToUpdate |= rm;

  needToUpdate = true;
  }

void SmallGraphicsObject::setSize(float ix, float iy, float iz) {
  if( sx==ix && sy==iy && sz==iz )
    return;

  sx = ix;
  sy = iy;
  sz = iz;

  needToUpdate =  visible;
  }

void SmallGraphicsObject::setRotation(float x, float z) {
  if( ax==x && az==z )
    return;

  ax = x;
  az = z;

  needToUpdate = visible;
  }

float SmallGraphicsObject::angleX() {
  return ax;
  }

float SmallGraphicsObject::angleZ() {
  return az;
  }

const Tempest::ModelBounds SmallGraphicsObject::bounds() const {
  return bds;
  }

float SmallGraphicsObject::x() const {
  return mx;
  }

float SmallGraphicsObject::y() const {
  return my;
  }

float SmallGraphicsObject::z() const {
  return mz;
  }

float SmallGraphicsObject::sizeX() const {
  return sx;
  }

float SmallGraphicsObject::sizeY() const {
  return sy;
  }

float SmallGraphicsObject::sizeZ() const {
  return sz;
  }

void SmallGraphicsObject::update() {
  if( needToUpdate ){
    applyTransform();

    TerrainChunk::PolishView& vx = chunk();

    vx.obj.setModel( Model() );
    vx.obj.setModel( game.resources().model(vx.geometry) );
    }

  needToUpdate = false;
  }

void SmallGraphicsObject::updateFull() {
  TerrainChunk::PolishView& vx = chunk();

  if( !vx.needToUpdate )
    return;

  vx.geometry.hasIndex = true;

  glocation = vx.geometry.vertex.size();
  ilocation = vx.geometry.index.size();

  vx.geometry.vertex.resize( vx.geometry.vertex.size() + model->vertex.size() );
  vx.geometry.index .resize( vx.geometry.index .size() + model->index .size() );

  applyTransform();
  }

void SmallGraphicsObject::setVisible(bool v) {
  if( visible==v )
    return;

  if( v ){
    chunkBase().polishObj.push_back( this );
    } else {
    remove( chunkBase().polishObj, this );
    }

  TerrainChunk::PolishView *c = &chunk();
  c->needToUpdate = true;

  visible = v;
  }

Tempest::Matrix4x4 SmallGraphicsObject::transform() const {
  return transformV;
  }

void SmallGraphicsObject::applyTransform() {
  TerrainChunk::PolishView& vx = chunk();

  Tempest::Matrix4x4& mat = transformV;

  mat.identity();
  mat.translate( mx - vx.posX, my - vx.posY, mz );

  mat.rotate( ax, 1, 0, 0 );
  mat.rotate( az, 0, 0, 1 );

  mat.scale( sx,sy,sz );

  MVertex *v = &vx.geometry.vertex[glocation];
  float x, y, z, w = 1;

  for( size_t i=0; i<model->vertex.size(); ++i, ++v ){
    const MVertex & s = model->vertex[i];
    *v = s;
    mat.project( s.x,
                 s.y,
                 s.z - vx.zView/sz,
                 1, x, y, z, w );

    v->x = x;
    v->y = y;
    v->z = z;

    mat.project( s.nx, s.ny, s.nz, 0, x, y, z, w );
    v->nx = x;
    v->ny = y;
    v->nz = z;

    mat.project( s.bx, s.by, s.bz, 0, x, y, z, w );
    v->bx = x;
    v->by = y;
    v->bz = z;
    }

  uint16_t * id = &vx.geometry.index[ ilocation ];
  for( size_t i=0; i<model->index.size(); ++i, ++id ){
    *id = glocation+model->index[i];
    }

  needToUpdate = false;
  }

TerrainChunk::PolishView &SmallGraphicsObject::chunk() {
  TerrainChunk &c = chunkBase();

  for( size_t i=0; i<c.polish.size(); ++i )
    if( c.polish[i]->baseView==&view &&
        fabs(c.polish[i]->zView - z()) < 0.001 ){
      return *c.polish[i].get();
      }

  TerrainChunk::PolishView *vx = new TerrainChunk::PolishView(scene, &view);
  vx->zView = z();
  vx->obj.setPosition( c.x, c.y, z());
  vx->posX = c.x;
  vx->posY = c.y;

  game.setupMaterials( vx->obj, view, Tempest::Color() );
  c.polish.push_back( std::shared_ptr<TerrainChunk::PolishView>(vx) );
  glocation = 0;

  if( model ){
    vx->geometry.vertex.resize( model->vertex.size() );
    vx->geometry.index .resize( model->index.size()  );
    }

  return *vx;
  }

TerrainChunk &SmallGraphicsObject::chunkBase() {
  return chunkBase(mx,my);
  }

TerrainChunk &SmallGraphicsObject::chunkBase(float mx, float my) {
  int tx = World::coordCastD(mx)/Terrain::quadSize;
  int ty = World::coordCastD(my)/Terrain::quadSize;

  tx = (tx*t->chunks.width())/t->width();
  ty = (ty*t->chunks.height())/t->height();

  tx = clamp( tx, 0, t->chunks.width()-1 );
  ty = clamp( ty, 0, t->chunks.height()-1 );

  t->chunks[tx][ty].x = World::coordCast(tx*Terrain::chunkSize*Terrain::quadSize);
  t->chunks[tx][ty].y = World::coordCast(ty*Terrain::chunkSize*Terrain::quadSize);

  return t->chunks[tx][ty];
  }
