#ifndef MAINMENU_H
#define MAINMENU_H

#include "modalwindow.h"

class Resource;
class Button;

class MainMenu : public ModalWindow {
  public:
    MainMenu( Resource& res, Widget *owner, bool startM = true );
    ~MainMenu();

  private:
    Button *button(Resource& res, const std::wstring &s,
                   void (MainMenu::*f)() );

    class Btn;

    void startMap( const std::string& mx );
    void start();
    void continueGame();
    void showOptions();
    void rate();

    Resource &res;

    void showAds(bool s);
  };

#endif // MAINMENU_H
