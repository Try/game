#include "smallobjectsview.h"

#include "landscape/terrain.h"
#include "game.h"
#include "algo/algo.h"

SmallGraphicsObject::SmallGraphicsObject( MyGL::Scene &s,
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

  glocation    = -1;
  needToUpdate = false;

  chunkBase().polishObj.push_back( this );
  chunk().needToUpdate = true;
  }

SmallGraphicsObject::~SmallGraphicsObject() {
  remove( chunkBase().polishObj, this );
  chunk().needToUpdate = true;
  }

void SmallGraphicsObject::setModel( const Model &m ) {
  bds = m.bounds();

  model.vertex.resize( m.size()*3 );
  m.vertexes().get( model.vertex.begin(),
                    model.vertex.end(),
                    0 );

  needToUpdate = true;
  chunk().needToUpdate = true;
  }

void SmallGraphicsObject::setPosition(float ix, float iy, float iz) {
  bool rm = ( &chunkBase(mx,my)!=&chunkBase(ix,iy) );

  if( rm )
    remove( chunkBase().polishObj, this );

  TerrainChunk::PolishView *v1 = &chunk();
  v1->needToUpdate |= rm;

  mx = ix;
  my = iy;
  mz = iz;

  if( rm )
    chunkBase().polishObj.push_back( this );

  TerrainChunk::PolishView *v2 = &chunk();
  v2->needToUpdate |= rm;

  needToUpdate = true;
  }

void SmallGraphicsObject::setSize(float ix, float iy, float iz) {
  sx = ix;
  sy = iy;
  sz = iz;

  needToUpdate = true;
  }

void SmallGraphicsObject::setRotation(float x, float z) {
  if( ax==x && az==z )
    return;

  ax = x;
  az = z;

  needToUpdate = true;
  }

float SmallGraphicsObject::angleZ() {
  return az;
  }

const MyGL::ModelBounds SmallGraphicsObject::bounds() const {
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

  glocation = vx.geometry.vertex.size();
  vx.geometry.vertex.resize( vx.geometry.vertex.size() + model.vertex.size() );

  applyTransform();
  }

void SmallGraphicsObject::applyTransform() {
  TerrainChunk::PolishView& vx = chunk();

  MyGL::Matrix4x4 mat;

  mat.identity();
  mat.translate( mx, my, mz );

  mat.scale( sx,sy,sz );

  mat.rotate( ax, 1, 0, 0 );
  mat.rotate( az, 0, 0, 1 );

  Model::Vertex *v = &vx.geometry.vertex[glocation];
  double x, y, z, w = 1;

  for( size_t i=0; i<model.vertex.size(); ++i, ++v ){
    Model::Vertex & s = model.vertex[i];
    *v = s;
    mat.project( s.x, s.y, s.z, 1, x, y, z, w );

    v->x = x;
    v->y = y;
    v->z = z;

    mat.project( s.normal[0], s.normal[1], s.normal[2], 0, x, y, z, w );

    v->normal[0] = x;
    v->normal[1] = y;
    v->normal[2] = z;
    }

  needToUpdate = false;
  }

TerrainChunk::PolishView &SmallGraphicsObject::chunk() {
  TerrainChunk &c = chunkBase();

  for( size_t i=0; i<c.polish.size(); ++i )
    if( c.polish[i]->baseView==&view ){
      return *c.polish[i].get();
      }

  TerrainChunk::PolishView *vx = new TerrainChunk::PolishView(scene, &view);
  game.setupMaterials( vx->obj, view, MyGL::Color() );
  c.polish.push_back( std::shared_ptr<TerrainChunk::PolishView>(vx) );
  glocation = 0;
  vx->geometry.vertex.resize( model.vertex.size() );

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

  return t->chunks[tx][ty];
  }
