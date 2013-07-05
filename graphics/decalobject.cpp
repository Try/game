#include "decalobject.h"

#include "landscape/terrain.h"
#include "game.h"
#include "algo/algo.h"

#include <cmath>

DecalObject::DecalObject(Scene &s,
                          Game &g,
                          Terrain &t,
                          const ProtoObject::View &v,
                          const ProtoObject::View *vdec )
  :PacketObject(s, g, t, v), vDecal(vdec) {
  chunk().needToUpdate = true;
  }

DecalObject::~DecalObject() {
  chunk().needToUpdate = true;
  }

void DecalObject::updateFull() {
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

void DecalObject::applyTransform() {
  TerrainChunk::PolishView& vx = chunk();

  updateTransform();

  Tempest::Matrix4x4 mat = localTransform(vx);

  MVertex *v = &vx.geometry.vertex[glocation];
  float x, y, z, w = 1, tu, tv;

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

    float tz = t->heightAt(x+vx.posX, y+vx.posY);
    if( z < tz ){//s.z - vx.zView/sizeZ() <=0 ){
      //z = tz;

      v->nx = 0;
      v->ny = 0;
      v->nz = 1;

      v->bx = 1;
      v->by = 0;
      v->bz = 0;
      }

    t->mkTexCoord(tu, tv, x+vx.posX, y+vx.posY);
    v->u = tu;//(x+vx.posX)*0.01;
    v->v = tv;//(y+vx.posY)*0.01;

    mat.project( v->nx, v->ny, v->nz, 0, x, y, z, w );
    v->nx = x;
    v->ny = y;
    v->nz = z;

    mat.project( v->bx, v->by, v->bz, 0, x, y, z, w );
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

TerrainChunk::PolishView &DecalObject::chunk() {
  TerrainChunk &c = chunkBase();
  vDecal = t->viewAt( x(), y() );

  for( size_t i=0; i<c.polish.size(); ++i )
    if( c.polish[i]->baseView == vDecal &&
        (isBlush || fabs(c.polish[i]->zView - z()) < 0.001)){
      return *c.polish[i].get();
      }

  TerrainChunk::PolishView *vx = new TerrainChunk::PolishView(scene, vDecal);
  vx->zView   = z();
  vx->obj.setPosition( c.x, c.y, z());
  vx->posX    = c.x;
  vx->posY    = c.y;
  //vx->isDecal = decal;

  game.setupMaterials( vx->obj, *vDecal, Tempest::Color() );
  c.polish.push_back( std::shared_ptr<TerrainChunk::PolishView>(vx) );
  glocation = 0;

  if( model ){
    vx->geometry.vertex.resize( model->vertex.size() );
    vx->geometry.index .resize( model->index.size()  );
    }

  return *vx;
  }

