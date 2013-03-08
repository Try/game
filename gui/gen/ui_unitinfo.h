#include <Tempest/Widget>

class Resource;

class Button;
class Panel;
class LineEdit;
class ScroolWidget;

namespace UI{ 

class UnitInfo {
  public: 
    void setupUi( Tempest::Widget*, Resource & res );
    Tempest::Widget * priview;
    Panel * hpBox;
    LineEdit * hp;
    LineEdit * caption;
    Tempest::Widget * status;
};
} 
