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

    bool needToUpdate;
  };

#endif // TERRAINCHUNK_H
