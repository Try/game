#ifndef SMALLOBJECTSVIEW_H
#define SMALLOBJECTSVIEW_H

#include "packetobject.h"

class SmallGraphicsObject : public PacketObject {
  public:
    SmallGraphicsObject( Scene &s,
                         Game & g,
                         Terrain & t,
                         const ProtoObject::View & view );
    ~SmallGraphicsObject();

    void updateFull();
  private:
    void applyTransform();
    TerrainChunk::PolishView& chunk();
  };

#endif // SMALLOBJECTSVIEW_H
