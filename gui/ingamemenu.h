#ifndef INGAMEMENU_H
#define INGAMEMENU_H

#include "modalwindow.h"
#include "graphics/paintergui.h"

class InGameMenu : public ModalWindow {
  public:
    InGameMenu(Resource &res, MyWidget::Widget* );

    MyWidget::signal<> save, load, quit;

  };

#endif // INGAMEMENU_H
