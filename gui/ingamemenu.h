#ifndef INGAMEMENU_H
#define INGAMEMENU_H

#include "modalwindow.h"
#include "graphics/paintergui.h"

class InGameMenu : public ModalWindow {
  public:
    InGameMenu(Resource &res, Tempest::Widget* );

    Tempest::signal<> save, load, quit;

  };

#endif // INGAMEMENU_H
