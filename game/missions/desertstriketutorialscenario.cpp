#include "desertstriketutorialscenario.h"

#include "desertstrikescenariowidgets.h"
#include "gui/hintsys.h"

#include "gamesettings.h"

DesertStrikeTutorialScenario::DesertStrikeTutorialScenario( Game &game,
                                                            MainGui &ui,
                                                            BehaviorMSGQueue &msg)
  :DesertStrikeScenario(game, ui, msg), timer(0){
  player(1).units["pikeman"] = 0;
  player(2).units["pikeman"] = 0;

  player(1).setGold(175);
  player(2).setGold(175);

  std::fill( panelHint, panelHint+3, false );
  state = stGame;

  casHint  = 0;
  farmHint = 0;
  }

void DesertStrikeTutorialScenario::onStartGame() {
  auto d = GameSettings::difficulty;
  GameSettings::difficulty = 0;
  DesertStrikeScenario::onStartGame();
  GameSettings::difficulty = d;

  Hint *h = new Hint(res, mainWidget, game);
  h->onDestroy.bind(this, &DesertStrikeTutorialScenario::showBuyUnitHint);
  game.pause(1);
  }

void DesertStrikeTutorialScenario::setupUI(InGameControls *mainWidget, Resource &res) {
  auto sm = GameSettings::smallMenu;
  GameSettings::smallMenu = 0;
  DesertStrikeScenario::setupUI(mainWidget, res);
  GameSettings::smallMenu = sm;

  bool units[3][4] = { {1,1,1,1},{},{} };
  buyUnitPanel->setup(1, units);
  upgradePanel->setVisible(0);
  }

void DesertStrikeTutorialScenario::showBuyUnitHint( Tempest::Widget* ) {
  state = stBuyArmy;
  }

void DesertStrikeTutorialScenario::onUnitToBuy( const std::string & ) {
  if( state==stBuyArmy )
    state = stAcceptUnit;
  }

void DesertStrikeTutorialScenario::onUnitHired(const std::string &) {
  if( state==stAcceptUnit ){
    timer = Game::ticksPerSecond*5;
    state = stGoldInfo;
    disableUnitBlock();
    }
  }

void DesertStrikeTutorialScenario::onPanelChoised(int p) {
  static const std::wstring ch[] = {
    L"$(desertstrike/tutorial/castle_panel)",
    L"$(desertstrike/tutorial/units_panel)",
    L"$(desertstrike/tutorial/grade_panel)"
    };

  if( state==stGame && !panelHint[p] ){
    panelHint[p] = 1;
    gameHint = ch[p];
    timer    = Game::ticksPerSecond*4;
    }
  }

void DesertStrikeTutorialScenario::disableUnitBlock() {
  bool units[3][4] = { {1,1,1,1},{1,1,1,1},{1,1,1,1} };
  buyUnitPanel->setup(1, units);
  upgradePanel->setVisible(1);
  }

void DesertStrikeTutorialScenario::tick() {
  switch( state ){
    case stBuyArmy:
      HintSys::setHint( L"$(desertstrike/tutorial/get_army)",
                        Tempest::Rect(mainWidget->w(), mainWidget->h()-220,200,200) );
      break;

    case stAcceptUnit:
      player().setGold(175);
      HintSys::setHint( L"$(desertstrike/tutorial/accept_buy)",
                        Tempest::Rect(0, mainWidget->h()-220,200,200) );
      break;

    case stGoldInfo:
      --timer;
      HintSys::setHint( L"$(desertstrike/tutorial/gold)",
                        Tempest::Rect(mainWidget->w()-100, 0,200,200) );
      if( timer<=0 ){
        timer = Game::ticksPerSecond*3;
        state = stCenter;
        }
      DesertStrikeScenario::tick();
      break;

    case stCenter:
      --timer;
      HintSys::setHint( L"$(desertstrike/tutorial/center)",
                        Tempest::Rect(mainWidget->w()/2-100, 0,200,200) );
      if( timer<=0 ){
        state = stGame;
        }
      DesertStrikeScenario::tick();
      break;

    case stGame:
      if( gameHint.size() ){
        HintSys::setHint( gameHint,
                          Tempest::Rect(mainWidget->w()-300,
                                        mainWidget->h()/2-100,200,200) );
        timer--;
        if( timer<=0 )
          gameHint.clear();
        }
      DesertStrikeScenario::tick();
      break;
    }
  }

void DesertStrikeTutorialScenario::onBuildingToBuy(const std::string &u) {
  if( state==stGame && u=="castle" && !casHint ){
    casHint = 1;
    gameHint = L"$(desertstrike/tutorial/castle)";
    timer    = Game::ticksPerSecond*4;
    }

  if( state==stGame && u=="farm" && !farmHint ){
    farmHint = 1;
    gameHint = L"$(desertstrike/tutorial/farm)";
    timer    = Game::ticksPerSecond*4;
    }
  }
