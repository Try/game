#ifndef DESERTSTRIKESCENARIO_H
#define DESERTSTRIKESCENARIO_H

#include "game/scenario.h"
#include <map>

class Game;
class MainGui;

class DesertStrikeScenario  : public Scenario {
  public:
    DesertStrikeScenario(Game &game,
                          MainGui & ui  , BehaviorMSGQueue &msg);

  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void customEvent( const std::vector<char> & m );
    void onUnitDied(GameObject &obj);

    void setupUI(InGameControls *mainWidget, Resource &res);
    Tempest::Widget *createConsole( InGameControls *mainWidget,
                                    BehaviorMSGQueue & q );

  private:
    void tick();
    int  tNum, interval;

    virtual void onStartGame();

    Tempest::Point mpos;
    F3             mpos3d;
    bool           isMouseTracking;

    using Scenario::showMenu;
    using Scenario::showFormBuilder;
    using Scenario::toogleEditPanel;
    using Scenario::toogleSettingsPanel;

    struct BuyButton;
    struct BuyUnitPanel;

    struct Minimap;
    Minimap        *mmap;

    struct PlInfo{
      std::map<std::string, int> units, realCount;
      };

    PlInfo plC[2];

    void mkUnits(int pl, int x, int y, int tgX, int tgY);
  };

#endif // DESERTSTRIKESCENARIO_H
