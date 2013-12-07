#ifndef MAINMENU_H
#define MAINMENU_H

#include "modalwindow.h"
#include <Tempest/Timer>

class Resource;
class Game;
class Button;

class MainMenu : public ModalWindow {
  public:
    MainMenu( Game &game, Resource& res, Widget *owner, bool startM = true );
    ~MainMenu();

    static int  adsHeight();
  private:
    Button *button(Resource& res, const std::wstring &s,
                   void (MainMenu::*f)() );

    class Btn;

    void paintEvent(Tempest::PaintEvent &e);

    void startMap(const std::wstring &mx );
    void startMapExt(const std::wstring &mx );
    void tutorial();
    void start();
    void continueGame();
    void showOptions();
    void rate();
    void showHelp();

    void updateParticles();
    void closeEvent(Tempest::CloseEvent &e);

    Resource &res;
    Game &game;

    Tempest::Sprite logo, efect;
    bool fbackgr;

    struct Particle{
      Tempest::Point pos, v;
      Tempest::Color color;
      float sz;
      };

    std::vector<Particle> par;

    void showAds(bool s);

    Tempest::Timer timer, efeTimer;
    Particle mkParticle();
  };

#endif // MAINMENU_H
