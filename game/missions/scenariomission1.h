#ifndef SCENARIOMISSION1_H
#define SCENARIOMISSION1_H

#include "game/scenario.h"

class Game;
class MainGui;

class ScenarioMission1 : public Scenario {
  public:
    ScenarioMission1( Game& game, MainGui & ui );
    ~ScenarioMission1();

  protected:
    void onStartGame();

  private:
    MainGui & ui;
    Game& game;

    class IntroWidget;
    IntroWidget *intro;

    void updateView();
    void closeIntro();
  };

#endif // SCENARIOMISSION1_H
