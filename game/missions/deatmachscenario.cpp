#include "deatmachscenario.h"

#include "gui/maingui.h"

DeatmachScenario::DeatmachScenario( Game& g, MainGui & ui, BehaviorMSGQueue &msg )
  :Scenario(g,ui, msg) {
  }

bool DeatmachScenario::isCampagin(){
  return false;
  }

