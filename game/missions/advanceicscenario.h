#ifndef ADVANCEICSCENARIO_H
#define ADVANCEICSCENARIO_H

#include "desertstrikescenario.h"

class AdvanceICScenario : public DesertStrikeScenario {
  public:
    AdvanceICScenario( Game &game,
                        MainGui & ui,
                        BehaviorMSGQueue &msg );

    void tick();
  };

#endif // ADVANCEICSCENARIO_H
