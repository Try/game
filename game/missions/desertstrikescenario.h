#ifndef DESERTSTRIKESCENARIO_H
#define DESERTSTRIKESCENARIO_H

#include "game/scenario.h"
#include <map>
#include "gui/minimapview.h"
#include "game/player.h"

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

    void toogleCameraMode();
  private:
    void tick();
    int  tNum, interval;

    struct ScaleCtrl{
      Tempest::Point  m0,  m1;
      Tempest::Point rm0, rm1;
      bool  isScaleMode;
      float d0, mouseD0;
      float angle0;
      };

    ScaleCtrl sctrl;

    GameObject * unitToView;
    bool hasVTracking;
    float cameraSpeed;
    void cancelTracking(float, float, Tempest::Event::MouseButton, MiniMapView::Mode);

    virtual void onStartGame();

    Tempest::Point mpos;
    F3             mpos3d;
    Tempest::Point mpressPos;
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
    struct CentralPanel;

    struct GoldButton;

    struct Minimap;
    Minimap        *mmap;
    BuyUnitPanel   *buyUnitPanel;
    CentralPanel   *cen;    

    static const char * units[3][4];

    struct DPlayer:Player{
      DPlayer( int num ):Player(num),
        atkGrade(0),
        armorGrade(0),
        castleGrade(0),
        economyGrade(0),
        aiTick(0){}

      std::map<std::string, int> units, realCount;
      int atkGrade,    armorGrade;
      int castleGrade, economyGrade;

      int getParam( const std::string& p ) const;
      std::vector<GameObject*>& selected();

      struct QElement{
        int btime, maxBTime;
        std::string    name;
        };
      std::vector<QElement> queue;

      int aiTick;
      bool isInQueue( const char* ch );
      };

    int   plCenter;
    bool  revPlPos;
    float moveZ;

    std::string spellToCast;
    Spell::Mode spellMode;

    void mkUnits(int pl, int x, int y, int tgX, int tgY, bool rev);
    void aiTick(int pl);
    void updateCenterOwner();

    void grade(DPlayer &pl, DPlayer::QElement &e);
    bool isTestRun;

    virtual Player* createPlayer();
    DPlayer& player(int i);
    DPlayer& player();

    int tierOf( const char* u );

    void setupTopUi( Resource &res, Tempest::Widget *ow );
    void toogleEditPanel();
  };

#endif // DESERTSTRIKESCENARIO_H
