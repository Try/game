#include <MyWidget/Widget>

class Resource;

class Button;
class Panel;
class LineEdit;
class ScroolWidget;

namespace UI{ 

class UnitInfo {
  public: 
    void setupUi( MyWidget::Widget*, Resource & res );
    MyWidget::Widget * priview;
    Panel * hpBox;
    LineEdit * hp;
    LineEdit * caption;
    MyWidget::Widget * status;
};
} 
