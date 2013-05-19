#ifndef DEATMACHSCENARION_H
#define DEATMACHSCENARION_H

#include "game/scenario.h"

class DeatmachScenarion  : public Scenario {
  public:
    DeatmachScenarion(Game& game, MainGui & ui , BehaviorMSGQueue &msg);
    virtual bool isCampagin();

  };

#endif // DEATMACHSCENARION_H
