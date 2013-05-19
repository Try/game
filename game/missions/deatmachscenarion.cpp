#include "deatmachscenarion.h"

#include "gui/maingui.h"

DeatmachScenarion::DeatmachScenarion( Game& g, MainGui & ui, BehaviorMSGQueue &msg )
  :Scenario(g,ui, msg) {
  }

bool DeatmachScenarion::isCampagin(){
  return false;
  }

