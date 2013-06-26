#ifndef SMALLOBJECTSVIEW_H
#define SMALLOBJECTSVIEW_H

#include "resource.h"
#include "game/protoobject.h"

#include <Tempest/GraphicObject>
#include "landscape/terrainchunk.h"

class Terrain;
class Game;

class SmallGraphicsObject {
  public:
    SmallGraphicsObject( Scene &s,
                         Game & g,
                         Terrain & t,
                         const ProtoObject::View & view );
    ~SmallGraphicsObject();

    SmallGraphicsObject( const SmallGraphicsObject& other ) = delete;
    SmallGraphicsObject& operator = ( const SmallGraphicsObject& other ) = delete;

    void setModel(const Model& m , const std::string &key);

    void setPosition( float x, float y, float z );
    void setSize( float x, float y, float z );
    const Tempest::ModelBounds bounds() const;

    const ProtoObject::View & view;

    float x() const;
    float y() const;
    float z() const;

    float sizeX() const;
    float sizeY() const;
    float sizeZ() const;

    void setRotation( float ax, float az );
    float angleX();
    float angleZ();

    void update();
    void updateFull();
    void updateDecal( const Terrain& t );

    void setVisible( bool v );

    Tempest::Matrix4x4 transform() const;
  private:
    Terrain * t;

    const Model::Raw* model;
    Tempest::ModelBounds bds;
    Scene &scene;

    Game & game;
    float  mx, my, mz, sx, sy, sz, ax, az;

    bool needToUpdate, visible;

    void applyTransform();
    Tempest::Matrix4x4 transformV;

    size_t glocation, ilocation;

    TerrainChunk::PolishView& chunk();
    TerrainChunk& chunkBase();
    TerrainChunk& chunkBase( float mx, float my );
  };

#endif // SMALLOBJECTSVIEW_H
