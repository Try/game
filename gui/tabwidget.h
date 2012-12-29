#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <MyWidget/Widget>
#include <vector>
#include "panel.h"

class Resource;
class Panel;
class Button;

class TabWidget : public Panel {
  public:
    TabWidget( Resource& res );
    ~TabWidget();

    void addTab( Widget* );
    void setCurrentTab( int id );

    MyWidget::signal<int> onTabChanged;
  private:
    using Widget::layout;

    struct MPanel;
    struct MBtn;
    Panel *p;

    std::vector<Widget*> tabs;
    std::vector<MBtn*>   btns;
    Resource& res;
    Widget* wbtns;
  };

#endif // TABWIDGET_H
