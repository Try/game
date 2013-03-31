#include "terrainchunk.h"

#include "graphics/smallobjectsview.h"

TerrainChunk::TerrainChunk() {
  needToUpdate = true;
  }

void TerrainChunk::update( Resource & res ) {
  bool wrk = false;

  for( size_t i=0; i<polish.size(); ++i )
    if( polish[i]->needToUpdate ){
      Model::Raw &r = polish[i]->geometry;
      r.vertex.reserve( r.vertex.size() );
      r.vertex.resize(0);

      r.index.reserve( r.index.size() );
      r.index.resize(0);

      wrk = true;
      }

  if( wrk ){
    for( size_t i=0; i<polishObj.size(); ++i ){
      polishObj[i]->updateFull();
      }

    for( size_t i=0; i<polish.size(); ++i )
      if( polish[i]->needToUpdate ){
        polish[i]->geometry.vertex.reserve( 0 );
        polish[i]->obj.setModel( Model() );
        polish[i]->obj.setModel( res.model(polish[i]->geometry) );

        polish[i]->needToUpdate = false;
        }
    }

  for( size_t i=0; i<polishObj.size(); ++i ){
    polishObj[i]->update();
    }
  }

