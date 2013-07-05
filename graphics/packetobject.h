#ifndef PACKETOBJECT_H
#define PACKETOBJECT_H

#include "resource.h"
#include "game/protoobject.h"

#include <Tempest/GraphicObject>
#include "landscape/terrainchunk.h"

class Terrain;
class Game;

class PacketObject {
  public:
    PacketObject( Scene &s,
                  Game & g,
                  Terrain & t,
                  const ProtoObject::View & v );
    virtual ~PacketObject();

    PacketObject( const PacketObject& other ) = delete;
    PacketObject& operator = ( const PacketObject& other ) = delete;

    void setModel(const Model& m , const std::string &key);

    void setPosition( float x, float y, float z );
    void setSize( float x, float y, float z );
    const Tempest::ModelBounds bounds() const;

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

    void setVisible( bool v );

    Tempest::Matrix4x4 transform() const;

    const ProtoObject::View & view;
  protected:
    Terrain * t;
    bool isBlush;

    const Model::Raw* model;
    Tempest::ModelBounds bds;
    Scene &scene;

    Game & game;

    virtual void applyTransform() = 0;

    size_t glocation, ilocation;

    virtual TerrainChunk::PolishView& chunk() = 0;
    TerrainChunk& chunkBase();
    TerrainChunk& chunkBase( float mx, float my );

    void updateTransform();
    Tempest::Matrix4x4 localTransform(TerrainChunk::PolishView &vx) const;

    bool needToUpdate;

  private:
    float  mx, my, mz, sx, sy, sz, ax, az;
    bool visible;
    Tempest::Matrix4x4 transformV;
  };

#endif // PACKETOBJECT_H
