#ifndef SCENARIOMISSION1_H
#define SCENARIOMISSION1_H

#include "game/scenario.h"
#include <MyWidget/signal>

class Game;
class MainGui;

class ScenarioMission1 : public Scenario {
  public:
    ScenarioMission1( Game& game, MainGui & ui );
    ~ScenarioMission1();

  protected:
    void onStartGame();
    void onItemEvent( GameObject & obj );
    void tick();
    void restartGame();

    const std::vector<MissionTaget>& tagets();

  private:
    MainGui & ui;
    Game& game;
    bool  isInGame;

    class IntroWidget;
    class EndScreenWidget;
    IntroWidget *intro;

    int lChest;
    std::vector<MissionTaget> mtagets;

    void updateView();
    void closeIntro();

    void start();
    void defeat();
    void winGame();
  };

#endif // SCENARIOMISSION1_H
