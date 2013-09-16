#ifndef MAINMENU_H
#define MAINMENU_H

#include "modalwindow.h"

class Resource;
class Button;

class MainMenu : public ModalWindow {
  public:
    MainMenu(Resource& res , Widget *owner);
    ~MainMenu();

  private:
    Button *button(Resource& res, const std::wstring &s,
                   void (MainMenu::*f)() );

    void start();
    void showOptions();

    Resource &res;

    void showAds(bool s);
  };

#endif // MAINMENU_H
