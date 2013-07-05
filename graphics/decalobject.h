#ifndef DECALOBJECT_H
#define DECALOBJECT_H

#include "packetobject.h"

class DecalObject : public PacketObject {
  public:
    DecalObject( Scene &s,
                 Game & g,
                 Terrain & t,
                 const ProtoObject::View & view,
                 const ProtoObject::View *vdec);
    ~DecalObject();

    void updateFull();
  private:
    void applyTransform();
    TerrainChunk::PolishView& chunk();

    const ProtoObject::View *vDecal;
  };

#endif // DECALOBJECT_H
