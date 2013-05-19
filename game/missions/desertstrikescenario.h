#ifndef DESERTSTRIKESCENARIO_H
#define DESERTSTRIKESCENARIO_H

#include "game/scenario.h"

class Game;
class MainGui;

class DesertStrikeScenario  : public Scenario {
  public:
    DesertStrikeScenario(Game &game,
                          MainGui & ui  , BehaviorMSGQueue &msg);

  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void setupUI(InGameControls *mainWidget, Resource &res);
  private:
    void tick();
    int  tNum, interval;

    virtual void onStartGame();

    Tempest::Point mpos;
    F3             mpos3d;
    bool           isMouseTracking;
  };

#endif // DESERTSTRIKESCENARIO_H
