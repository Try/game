#ifndef UI_INGAMEMENU_H
#define UI_INGAMEMENU_H

#include <Tempest/Widget>

class Resource;
class Button;

namespace UI{

class InGameMenu {
  public:
    void setupUi(Tempest::Widget*, Resource &res );
    Button * resume;
    Button * save;
    Button * load;
    Button * quit;
};
}

#endif // UI_INGAMEMENU_H
