#include <Tempest/Widget>

class Resource;

class Button;
class Panel;
class LineEdit;
class ScroolWidget;

namespace UI{ 

class LoadDialog {
  public: 
    void setupUi( Tempest::Widget*, Resource & res );
    Tempest::Widget * topWidget;
    Panel * topPanel;
    LineEdit * inputName;
    Tempest::Widget * central;
    ScroolWidget * items;
    Tempest::Widget * bottom;
    Button * accept;
    Button * cancel;
};
} 
