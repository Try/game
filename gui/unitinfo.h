#ifndef UNITINFO_H
#define UNITINFO_H

#include <Tempest/Widget>
#include "panel.h"

class Resource;
class GameObject;

class UnitInfo : public Tempest::Widget {
  public:
    UnitInfo( GameObject& obj, Resource &res);
    UnitInfo( Resource &res);

    void setup( GameObject * obj );
  private:
    GameObject* obj;
    Resource &  res;

    struct PanelBase;
    struct Production;
    struct Stats;
    struct Btn;
  };

#endif // UNITINFO_H
