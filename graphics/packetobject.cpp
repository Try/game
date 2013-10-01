#include "packetobject.h"

#include "landscape/terrain.h"
#include "game.h"
#include "algo/algo.h"

#include <cmath>

PacketObject::PacketObject( Scene &s,
                            Game &g,
                            Terrain &t,
                            const ProtoObject::View &v )
  : view(v), t(&t), scene(s), game(g) {
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

  isBlush = contains(v.materials, "blush");

  chunkBase().polishObj.push_back( this );
  }

PacketObject::~PacketObject() {
  remove( chunkBase().polishObj, this );
  }

void PacketObject::setModel( const Model &m, const std::string & key ) {
  bds = m.bounds();

  model = &game.resources().rawModel( key );
  needToUpdate = true;
  chunk().needToUpdate = true;
  }

void PacketObject::setPosition(float ix, float iy, float iz) {
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

void PacketObject::setSize(float ix, float iy, float iz) {
  if( sx==ix && sy==iy && sz==iz )
    return;

  sx = ix;
  sy = iy;
  sz = iz;

  needToUpdate =  visible;
  }

void PacketObject::setRotation(float x, float z) {
  if( ax==x && az==z )
    return;

  ax = x;
  az = z;

  needToUpdate = visible;
  }

float PacketObject::angleX() {
  return ax;
  }

float PacketObject::angleZ() {
  return az;
  }

const Tempest::ModelBounds PacketObject::bounds() const {
  return bds;
  }

float PacketObject::x() const {
  return mx;
  }

float PacketObject::y() const {
  return my;
  }

float PacketObject::z() const {
  return mz;
  }

float PacketObject::sizeX() const {
  return sx;
  }

float PacketObject::sizeY() const {
  return sy;
  }

float PacketObject::sizeZ() const {
  return sz;
  }

void PacketObject::update() {
  if( needToUpdate ){
    applyTransform();

    TerrainChunk::PolishView& vx = chunk();

    Model().setTo( vx.obj );
    game.resources().model(vx.geometry).setTo( vx.obj );
    }

  needToUpdate = false;
  }

void PacketObject::updateFull() {
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

void PacketObject::setVisible(bool v) {
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

Tempest::Matrix4x4 PacketObject::transform() const {
  return transformV;
  }

TerrainChunk &PacketObject::chunkBase() {
  return chunkBase(mx,my);
  }

TerrainChunk &PacketObject::chunkBase(float mx, float my) {
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

void PacketObject::updateTransform() {
  transformV.identity();
  transformV.translate( mx, my, mz );

  transformV.rotate( ax, 1, 0, 0 );
  transformV.rotate( az, 0, 0, 1 );

  transformV.scale( sx,sy,sz );
  }

Tempest::Matrix4x4 PacketObject::localTransform( TerrainChunk::PolishView& vx ) const {
  Tempest::Matrix4x4 mat;

  mat.identity();
  mat.translate( mx - vx.posX, my - vx.posY, mz );

  mat.rotate( ax, 1, 0, 0 );
  mat.rotate( az, 0, 0, 1 );

  mat.scale( sx,sy,sz );

  return mat;
  }
