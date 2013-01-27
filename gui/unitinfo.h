#ifndef UNITINFO_H
#define UNITINFO_H

#include <MyWidget/Widget>
#include "panel.h"

class Resource;
class GameObject;

class UnitInfo : public MyWidget::Widget {
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
