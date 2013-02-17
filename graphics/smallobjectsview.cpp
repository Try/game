#include "smallobjectsview.h"

#include "landscape/terrain.h"
#include "game.h"
#include "algo/algo.h"

#include <cmath>

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

  if( !vx.needToUpdate )
    return;

  glocation = vx.geometry.vertex.size();
  vx.geometry.vertex.resize( vx.geometry.vertex.size() + model->vertex.size() );

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

MyGL::Matrix4x4 SmallGraphicsObject::transform() const {
  return transformV;
  }

void SmallGraphicsObject::applyTransform() {
  TerrainChunk::PolishView& vx = chunk();

  MyGL::Matrix4x4& mat = transformV;

  mat.identity();
  mat.translate( mx, my, mz );

  mat.rotate( ax, 1, 0, 0 );
  mat.rotate( az, 0, 0, 1 );

  mat.scale( sx,sy,sz );

  Model::Vertex *v = &vx.geometry.vertex[glocation];
  double x, y, z, w = 1;

  for( size_t i=0; i<model->vertex.size(); ++i, ++v ){
    const Model::Vertex & s = model->vertex[i];
    *v = s;
    mat.project( s.x, s.y, s.z-vx.zView/sz, 1, x, y, z, w );

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
    if( c.polish[i]->baseView==&view &&
        fabs(c.polish[i]->zView - z()) < 0.001 ){
      return *c.polish[i].get();
      }

  TerrainChunk::PolishView *vx = new TerrainChunk::PolishView(scene, &view);
  vx->zView = z();
  vx->obj.setPosition(0,0, z());

  game.setupMaterials( vx->obj, view, MyGL::Color() );
  c.polish.push_back( std::shared_ptr<TerrainChunk::PolishView>(vx) );
  glocation = 0;

  if( model )
    vx->geometry.vertex.resize( model->vertex.size() );

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
