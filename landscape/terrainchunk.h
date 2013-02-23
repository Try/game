#ifndef TERRAINCHUNK_H
#define TERRAINCHUNK_H

#include <vector>
#include <memory>

#include "game/gameobjectview.h"
#include "graphics/scene.h"

struct TerrainChunk {
  public:
    TerrainChunk();

    struct View{
      std::shared_ptr<GameObjectView> view;
      };
    std::vector<View> landView;
    View waterView, fogView;

    struct PolishView {
      PolishView( Scene& s,
                  const ProtoObject::View *v ):obj(s), baseView(v){}

      GraphicObject obj;
      Model::Raw          geometry;

      const ProtoObject::View  *baseView;
      float zView;

      bool needToUpdate;
      };

    bool needToUpdate;
    std::vector< std::shared_ptr<PolishView> > polish;
    std::vector< SmallGraphicsObject* > polishObj;

    void update(Resource &res);
  };

#endif // TERRAINCHUNK_H
