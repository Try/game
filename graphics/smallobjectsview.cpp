#include "smallobjectsview.h"

#include "landscape/terrain.h"
#include "game.h"
#include "algo/algo.h"

#include <cmath>

SmallGraphicsObject::SmallGraphicsObject( Scene &s,
                                          Game &g,
                                          Terrain &t,
                                          const ProtoObject::View &v)
  :PacketObject(s, g, t, v) {
  chunk().needToUpdate = true;
  }

SmallGraphicsObject::~SmallGraphicsObject() {
  chunk().needToUpdate = true;
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

void SmallGraphicsObject::applyTransform() {
  TerrainChunk::PolishView& vx = chunk();

  updateTransform();

  Tempest::Matrix4x4 mat = localTransform(vx);

  MVertex *v = &vx.geometry.vertex[glocation];
  float x, y, z, w = 1;

  for( size_t i=0; i<model->vertex.size(); ++i, ++v ){
    const MVertex & s = model->vertex[i];
    *v = s;
    mat.project( s.x,
                 s.y,
                 s.z - vx.zView/sizeZ(),
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
        ( isBlush || fabs(c.polish[i]->zView - z()) < 0.001) ){
      return *c.polish[i].get();
      }

  TerrainChunk::PolishView *vx = new TerrainChunk::PolishView(scene, &view);
  vx->zView   = z();
  vx->obj.setPosition( c.x, c.y, z());
  vx->posX    = c.x;
  vx->posY    = c.y;
  //vx->isDecal = decal;

  //ProtoObject::View v = view;
  //remove(v.materials, "shadow_cast");

  game.setupMaterials( vx->obj, view, Tempest::Color() );
  c.polish.push_back( std::shared_ptr<TerrainChunk::PolishView>(vx) );
  glocation = 0;

  if( model ){
    vx->geometry.vertex.resize( model->vertex.size() );
    vx->geometry.index .resize( model->index.size()  );
    }

  return *vx;
  }

