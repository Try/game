#ifndef SMALLOBJECTSVIEW_H
#define SMALLOBJECTSVIEW_H

#include "resource.h"
#include "game/protoobject.h"

#include <MyGL/GraphicObject>
#include "landscape/terrainchunk.h"

class Terrain;
class Game;

class SmallGraphicsObject {
  public:
    SmallGraphicsObject( MyGL::Scene &s,
                         Game & g,
                         Terrain & t,
                         const ProtoObject::View & view );
    SmallGraphicsObject( const SmallGraphicsObject& other ) = delete;
    ~SmallGraphicsObject();

    SmallGraphicsObject& operator = ( const SmallGraphicsObject& other ) = delete;

    void setModel( const Model& m );

    void setPosition( float x, float y, float z );
    void setSize( float x, float y, float z );
    const MyGL::ModelBounds bounds() const;

    const ProtoObject::View & view;

    float x() const;
    float y() const;
    float z() const;

    float sizeX() const;
    float sizeY() const;
    float sizeZ() const;

    void setRotation( float ax, float az );
    float angleZ();

    void update();
    void updateFull();
  private:
    Terrain * t;

    Model::Raw model;
    MyGL::ModelBounds bds;
    MyGL::Scene &scene;

    Game & game;
    float  mx, my, mz, sx, sy, sz, ax, az;

    bool needToUpdate;

    void applyTransform();

    size_t glocation;

    TerrainChunk::PolishView& chunk();
    TerrainChunk& chunkBase();
    TerrainChunk& chunkBase( float mx, float my );
  };

#endif // SMALLOBJECTSVIEW_H
