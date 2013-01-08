#ifndef TERRAINCHUNK_H
#define TERRAINCHUNK_H

#include <vector>
#include <memory>

#include "game/gameobjectview.h"

struct TerrainChunk {
  public:
    TerrainChunk();

    struct View{
      std::shared_ptr<GameObjectView> view;
      };
    std::vector<View> landView;

    struct PolishView {
      PolishView( MyGL::Scene& s,
                  const ProtoObject::View *v ):obj(s), baseView(v){}

      MyGL::GraphicObject obj;
      Model::Raw          geometry;
      const ProtoObject::View  *baseView;

      bool needToUpdate;
      };

    bool needToUpdate;
    std::vector< std::shared_ptr<PolishView> > polish;
    std::vector< SmallGraphicsObject* > polishObj;

    void update(Resource &res);
  };

#endif // TERRAINCHUNK_H
