#ifndef MISSIONTARGETS_H
#define MISSIONTARGETS_H

#include <Tempest/Widget>
#include "graphics/paintergui.h"

class Resource;
class Game;

class MissionTargets : public Tempest::Widget {
  public:
    MissionTargets( Game &game, Resource & res );

  protected:
    void paintEvent(Tempest::PaintEvent &e);

  private:
    Tempest::Bind::UserTexture frame, ckFrame, ck;
    Tempest::Widget * box;

    Game & game;
    Resource & res;

    void setupTagets();
  };

#endif // MISSIONTARGETS_H
