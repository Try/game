#ifndef DEATMACHSCENARIO_H
#define DEATMACHSCENARIO_H

#include "game/scenario.h"

class DeatmachScenario  : public Scenario {
  public:
    DeatmachScenario(Game& game, MainGui & ui , BehaviorMSGQueue &msg);
    virtual bool isCampagin();

  };

#endif // DEATMACHSCENARIO_H
