#ifndef SCENARIO_H
#define SCENARIO_H

class GameObject;

#include <string>
#include <vector>
#include <memory>

#include <Tempest/Event>
#include <Tempest/Shortcut>

class Game;
class World;
class MainGui;

class BehaviorMSGQueue;
class GameSerializer;
class Resource;
class Button;
class EditTerrainPanel;
class MiniMapView;
class CommandsPanel;
class UnitList;

class ProtoObject;

struct F3{
  float data[3];
  };

namespace Tempest{
  class Widget;
  }

class InGameControls;
class Player;

struct Scenario {
    Scenario(Game& game, MainGui & ui, BehaviorMSGQueue &msg);
    virtual ~Scenario();

    virtual void onStartGame() {}
    virtual void tick();
    virtual void onRender();
    virtual void uiTick();
    virtual void onEndGame()   {}
    virtual void restartGame() {}

    virtual void onItemEvent( GameObject & obj )   { (void)obj; }

    virtual void serialize( GameSerializer & ){}
    virtual void customEvent( const std::vector<char> & ){}

    struct MissionTaget{
      bool done;
      std::wstring hint;
      };

    virtual const std::vector<MissionTaget>& tagets();
    virtual bool isCampagin() { return true; }

    virtual void mouseDownEvent( Tempest::MouseEvent &e );
    virtual void mouseUpEvent( Tempest::MouseEvent &e );
    virtual void mouseMoveEvent( Tempest::MouseEvent &e );
    virtual void mouseWheelEvent( Tempest::MouseEvent &e );

    virtual void keyDownEvent( Tempest::KeyEvent &e );
    virtual void keyUpEvent( Tempest::KeyEvent & e );

    virtual bool isSelectionRectTracking() const;

    F3 unProject(int x, int y, float destZ);
    F3 unProjectRz(int x, int y, float& destZ);
    F3 unProject(int x, int y);
    F3 project(float x, float y, float z);

    virtual void setupUI(InGameControls *mainWidget, Resource &res);

    virtual void setupMinimap(World *w);
    virtual void renderMinimap();

    virtual void onUnitDied( GameObject &obj );
    virtual void updateSelectUnits( const std::vector<GameObject *> &u );

    std::string name() const;

    virtual void addPlayer();
    virtual size_t plCount() const;
    virtual void setPlaylersCount( int c );

    virtual Player &player(int i);
    virtual Player &player();

    static void createMenu(Resource &res, Game &game, Tempest::Widget * w );
  protected:
    Game    & game;
    MainGui & gui;
    BehaviorMSGQueue & msg;

    bool mouseTracking, acceptMouseObj;
    int  selectionRectTracking;

    Tempest::Point curMPos;
    Tempest::KeyEvent::KeyType   lastKEvent;

    void updateMousePos( Tempest::MouseEvent &e );

    void moveCamera();

    Resource    & res;

    Tempest::Widget  *createSettingsPanel(Resource &res);
    EditTerrainPanel *createLandEdit( InGameControls *mainWidget,
                                     Resource & res );
    Tempest::Widget* createEditPanel(InGameControls *mainWidget, Resource &res);
    Tempest::Widget* createConsole( InGameControls *mainWidget,
                                   BehaviorMSGQueue & q );

    void toogleEditPanel();
    void toogleSettingsPanel();

    void showFormBuilder();
    void showMenu();

    Tempest::Shortcut showEditPanel, showSettings;

    Tempest::Widget *editPanel, *settingsPanel;
    Button *gold, *lim;

    InGameControls * mainWidget;
    MiniMapView    * minimap;

    virtual Player* createPlayer();
  private:
    CommandsPanel  * commands;
    UnitList       * units;
    std::vector< std::shared_ptr<Player> > players;

    int currPl;

    void setCurrPl( size_t i );
    void addEditorObject( const ProtoObject &p );

    struct AddUnitButton;
  };

#endif // SCENARIO_H
