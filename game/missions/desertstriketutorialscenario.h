#ifndef DESERTSTRIKETUTORIALSCENARIO_H
#define DESERTSTRIKETUTORIALSCENARIO_H

#include "desertstrikescenario.h"

class DesertStrikeTutorialScenario:public DesertStrikeScenario {
  public:
    DesertStrikeTutorialScenario( Game &game,
                                  MainGui & ui,
                                  BehaviorMSGQueue &msg );

  private:
     void onStartGame();
     void setupUI(InGameControls *mainWidget, Resource &res);

     void showBuyUnitHint( Tempest::Widget *w);

     enum State{
       stBuyArmy,
       stAcceptUnit,
       stGoldInfo,
       stCenter,
       stGame
       } state;

     int  timer;
     void tick();

     bool panelHint[3];
     bool casHint, farmHint;
     std::wstring gameHint;

     void onBuildingToBuy( const std::string& );
     void onUnitToBuy( const std::string& );
     void onUnitHired( const std::string& );
     void onPanelChoised(int p);

     void disableUnitBlock();
  };

#endif // DESERTSTRIKETUTORIALSCENARIO_H
