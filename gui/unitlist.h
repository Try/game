#ifndef UNITLIST_H
#define UNITLIST_H

#include <MyWidget/Widget>
#include "scroolwidget.h"

class Resource;
class GameObject;

class CommandsPanel;
class UnitView;
class UnitInfo;

class UnitList : public ScroolWidget {
  public:
    UnitList( CommandsPanel *panel,
              Resource &res,
              UnitView *uview,
              UnitInfo * uinf );

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
    UnitInfo *uinfo;
    CommandsPanel *cmd;

    std::vector<GameObject*> units;

    void onBtn( GameObject* );
  };

#endif // UNITLIST_H
