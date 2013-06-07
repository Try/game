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
    ~DesertStrikeScenario();

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

    struct NumButton;
    struct BuyButton;
    struct GradeButton;
    struct BuyUnitPanel;
    struct TranscurentPanel;
    struct UpgradePanel;
    struct SpellPanel;

    struct Minimap;
    Minimap        *mmap;
    BuyUnitPanel   *buyUnitPanel;

    struct PlInfo{
      PlInfo():atkGrade(0),
               armorGrade(0),
               castleGrade(0),
               economyGrade(0),
               btime(0),
               maxBTime(0){}

      std::map<std::string, int> units, realCount;
      int atkGrade,    armorGrade;
      int castleGrade, economyGrade;

      int getParam( const std::string& p ) const;

      std::vector<std::string> queue;
      int btime, maxBTime;
      };

    PlInfo plC[2];
    float moveZ;

    void mkUnits(int pl, int x, int y, int tgX, int tgY, bool rev);
    void aiTick(int pl);
  };

#endif // DESERTSTRIKESCENARIO_H
