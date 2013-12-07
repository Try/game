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
    DesertStrikeScenario( Game &game,
                          MainGui & ui,
                          BehaviorMSGQueue &msg );
    ~DesertStrikeScenario();

    static void createMenu(Resource &res, Game &game, Tempest::Widget * w );
    static Tempest::Size buttonOptimalSize;
    static int uiScale;
    struct UInfo;
  protected:
    virtual void onStartGame();

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void customEvent( const std::vector<char> & m );
    void onUnitDied(GameObject &obj);

    void setupUI(InGameControls *mainWidget, Resource &res);
    Tempest::Widget *createConsole( InGameControls *mainWidget,
                                    BehaviorMSGQueue & q );

    void toogleCameraMode();

    int  waveNumber() const;

    struct HightLight{
      Tempest::Rect rect;
      int alpha;
      };

    std::vector<HightLight> hightligt;

    void addHigtlight( const Tempest::Rect &rect );
    struct Hint;

    struct DPlayer:Player{
      DPlayer( int num ):Player(num),
        atkGrade(0),
        armorGrade(0),
        castleGrade(0),
        economyGrade(0),
        aiTick(0),
        dificultyLv(1){}

      std::map<std::string, int> units, realCount;
      int atkGrade,    armorGrade;
      int castleGrade, economyGrade;

      Tempest::Point spawnPoint;

      int getParam( const std::string& p ) const;
      std::vector<GameObject*>& selected();

      struct QElement{
        int btime, maxBTime;
        std::string    name;
        };
      std::vector<QElement> queue;

      int aiTick;
      int dificultyLv;
      bool isInQueue( const char* ch );
      };

    DPlayer& player(int i);
    DPlayer& player();

    void tick();
    void onRender();

    virtual void onBuildingToBuy( const std::string& ){}
    virtual void onUnitToBuy( const std::string& ){}
    virtual void onUnitHired( const std::string& ){}
    virtual void onPanelChoised(int p);

    void toogleMinimap(int page);

    struct UBtn;
    struct NumButton;
    struct BuyButton;
    struct GradeButton;
    struct BuyUnitPanel;
    struct TranscurentPanel;
    struct UpgradePanel;
    struct SpellPanel;
    struct CentralPanel;
    struct MiniBuyPanel;

    struct GoldButton;
    struct WinLoseScreen;

    struct Minimap;
    Panel          *mmapbox;
    Minimap        *mmap;
    BuyUnitPanel   *buyUnitPanel;
    MiniBuyPanel   *miniBuy;
    CentralPanel   *cen;
    SpellPanel     *spellPanel;
    UpgradePanel   *upgradePanel;

    static const char * units[3][4];
    int  tNum, interval;

    void spawn(const char* u, int pl,
                int x,
                int y, int tgX, int tgY);
    void moveCamera();
  private:
    static bool defaultMainMenu;

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

    Tempest::Point mpos;
    F3             mpos3d;
    Tempest::Point mpressPos;
    bool           isMouseTracking;
    bool           acceptSpell;

    void showWinLose();

    using Scenario::showMenu;
    using Scenario::showFormBuilder;
    using Scenario::toogleEditPanel;
    using Scenario::toogleSettingsPanel;

    int   plCenter;
    bool  revPlPos;
    float moveZ;

    struct WinAnim{
      bool isWinAnim;
      Tempest::Point viewPos;
      int timer;
      bool isWin;

      void setup( const Tempest::Point& p, bool win );
      } winAnim;

    std::string spellToCast;
    Spell::Mode spellMode;
    int  waveNum;

    void mkUnits(int pl, int x, int y, int tgX, int tgY);
    void aiTick(int pl);
    void updateCenterOwner();

    void grade(DPlayer &pl, DPlayer::QElement &e);
    bool isTestRun;

    virtual Player* createPlayer();

    int tierOf( const char* u );

    void setupTopUi( Resource &res, Tempest::Widget *ow );
    void toogleEditPanel();
    void showMainMenu( bool start );
  };

#endif // DESERTSTRIKESCENARIO_H
