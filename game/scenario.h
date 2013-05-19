#ifndef SCENARIO_H
#define SCENARIO_H

class GameObject;

#include <string>
#include <vector>
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

struct Scenario {
    Scenario(Game& game, MainGui & ui, BehaviorMSGQueue &msg);
    virtual ~Scenario(){}

    virtual void onStartGame() {}
    virtual void tick();
    virtual void uiTick();
    virtual void onEndGame()   {}
    virtual void restartGame() {}

    virtual void onItemEvent( GameObject & obj )   { (void)obj; }

    virtual void serialize( GameSerializer & ){}

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
    F3 unProject(int x, int y);
    F3 project(float x, float y, float z);

    virtual void setupUI(InGameControls *mainWidget, Resource &res);

    virtual void setupMinimap(World& w);
    virtual void renderMinimap();

    virtual void onUnitDied( GameObject &obj );
    virtual void updateSelectUnits( const std::vector<GameObject *> &u );
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
  private:
    Tempest::Shortcut showEditPanel, showSettings;

    Tempest::Widget *editPanel, *settingsPanel;
    Button *gold, *lim;

    MiniMapView    * minimap;
    InGameControls * mainWidget;
    CommandsPanel  * commands;
    UnitList       * units;

    int currPl;

    void setCurrPl( size_t i );
    void addEditorObject( const ProtoObject &p );

    void toogleEditPanel();
    void toogleSettingsPanel();

    Tempest::Widget  *createSettingsPanel(InGameControls *mainWidget, Resource &res);
    EditTerrainPanel *createLandEdit( InGameControls *mainWidget,
                                     Resource & res );
    Tempest::Widget* createEditPanel(InGameControls *mainWidget, Resource &res);
    Tempest::Widget* createConsole( InGameControls *mainWidget,
                                   BehaviorMSGQueue & q );
    void showFormBuilder();
    void showMenu();

    struct AddUnitButton;
  };

#endif // SCENARIO_H
