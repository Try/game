#include <MyWidget/Widget>

class Resource;

class Button;
class Panel;
class LineEdit;
class ScroolWidget;

namespace UI{ 

class LoadDialog {
  public: 
    void setupUi( MyWidget::Widget*, Resource & res );
    MyWidget::Widget * topWidget;
    Panel * topPanel;
    LineEdit * inputName;
    MyWidget::Widget * central;
    ScroolWidget * items;
    MyWidget::Widget * bottom;
    Button * accept;
    Button * cancel;
};
} 
