#ifndef MAINMENU_H
#define MAINMENU_H

#include "modalwindow.h"

class Resource;
class Game;
class Button;

class MainMenu : public ModalWindow {
  public:
    MainMenu( Game &game, Resource& res, Widget *owner, bool startM = true );
    ~MainMenu();

  private:
    Button *button(Resource& res, const std::wstring &s,
                   void (MainMenu::*f)() );

    class Btn;

    void paintEvent(Tempest::PaintEvent &e);

    void startMap(const std::wstring &mx );
    void tutorial();
    void start();
    void continueGame();
    void showOptions();
    void rate();

    Resource &res;
    Game &game;

    Tempest::Sprite logo;
    void showAds(bool s);
  };

#endif // MAINMENU_H
