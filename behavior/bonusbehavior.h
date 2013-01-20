#ifndef BONUSBEHAVIOR_H
#define BONUSBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"
#include "util/weakworldptr.h"
#include "gui/inputhook.h"

class BonusBehavior : public AbstractBehavior {
  public:
    BonusBehavior( GameObject & obj,
                   Behavior::Closure & c );

    void tick( const Terrain &  );

  private:
    GameObject & obj;
    static void lookOn( GameObject &tg, GameObject &obj );
    static void hillOn( GameObject &tg, GameObject &obj );
  };

#endif // BONUSBEHAVIOR_H
