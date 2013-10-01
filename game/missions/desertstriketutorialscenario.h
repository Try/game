#ifndef DESERTSTRIKETUTORIALSCENARIO_H
#define DESERTSTRIKETUTORIALSCENARIO_H

#include "desertstrikescenario.h"

class DesertStrikeTutorialScenario:public DesertStrikeScenario {
  public:
    DesertStrikeTutorialScenario( Game &game,
                                  MainGui & ui,
                                  BehaviorMSGQueue &msg );
  };

#endif // DESERTSTRIKETUTORIALSCENARIO_H
