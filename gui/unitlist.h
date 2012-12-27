#ifndef UNITLIST_H
#define UNITLIST_H

#include <MyWidget/Widget>
#include "scroolwidget.h"

class Resource;
class GameObject;

class CommandsPanel;
class UnitView;

class UnitList : public ScroolWidget {
  public:
    UnitList( CommandsPanel *panel, Resource &res,
              UnitView *uview );

    void setup( const std::vector<GameObject *> &u );
    void onUnitDied(GameObject &obj);

    MyWidget::signal<GameObject&> setCameraPos;
  private:
    struct View;
    struct Btn;
    struct Lay;

    std::vector<Btn*> btn;
    Resource &res;

    View * view;
    UnitView *uview;
    CommandsPanel *cmd;
  };

#endif // UNITLIST_H
