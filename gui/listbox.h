#ifndef LISTBOX_H
#define LISTBOX_H

#include "gui/button.h"

class Resource;
class MainGui;

class ListBox : public Button {
  public:
    ListBox( MainGui& gui, Resource & r );

  private:
    void showList();
    MainGui& gui;
    Resource &res;
  };

#endif // LISTBOX_H
