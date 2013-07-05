#ifndef TERRAINCHUNK_H
#define TERRAINCHUNK_H

#include <vector>
#include <memory>
#include "game/gameobjectview.h"
#include "graphics/scene.h"

class PacketObject;

struct TerrainChunk {
  public:
    TerrainChunk();

    struct View{
      std::shared_ptr<GameObjectView> view, shadow;
      };
    std::vector<View> landView;
    View waterView, fogView;

    float x,y;

    struct PolishView {
      PolishView( Scene& s,
                  const ProtoObject::View *v ):obj(s),
        baseView(v) {}

      GraphicObject obj;
      Model::Raw          geometry;

      const ProtoObject::View  *baseView;
      float zView;
      float posX, posY;

      bool needToUpdate;
      };

    bool needToUpdate;
    std::vector< std::shared_ptr<PolishView> > polish;
    std::vector< PacketObject* > polishObj;

    void update(Resource &res);
  };

#endif // TERRAINCHUNK_H
