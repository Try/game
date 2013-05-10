#ifndef DESERTSTRIKESCENARIO_H
#define DESERTSTRIKESCENARIO_H

#include "game/scenario.h"

class Game;
class MainGui;

class DesertStrikeScenario  : public Scenario {
  public:
    DesertStrikeScenario( Game &game,
                          MainGui & ui  );

  private:
    Game &game;
    MainGui & ui;

    void tick();
    int  tNum, interval;

    virtual void onStartGame();
  };

#endif // DESERTSTRIKESCENARIO_H
