#ifndef MISSIONTARGETS_H
#define MISSIONTARGETS_H

#include <Tempest/Widget>
#include <Tempest/Sprite>
#include "graphics/paintergui.h"

class Resource;
class Game;

class MissionTargets : public Tempest::Widget {
  public:
    MissionTargets( Game &game, Resource & res );

  protected:
    void paintEvent(Tempest::PaintEvent &e);

  private:
    Tempest::Sprite frame, ckFrame, ck;
    Tempest::Widget * box;

    Game & game;
    Resource & res;

    void setupTagets();
  };

#endif // MISSIONTARGETS_H
