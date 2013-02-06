#ifndef MISSIONTARGETS_H
#define MISSIONTARGETS_H

#include <MyWidget/Widget>
#include "graphics/paintergui.h"

class Resource;
class Game;

class MissionTargets : public MyWidget::Widget {
  public:
    MissionTargets( Game &game, Resource & res );

  protected:
    void paintEvent(MyWidget::PaintEvent &e);

  private:
    MyWidget::Bind::UserTexture frame, ckFrame, ck;
    MyWidget::Widget * box;

    Game & game;
    Resource & res;

    void setupTagets();
  };

#endif // MISSIONTARGETS_H
