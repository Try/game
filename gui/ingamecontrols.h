#ifndef INGAMECONTROLS_H
#define INGAMECONTROLS_H

#include <Tempest/Widget>
#include "graphics/paintergui.h"
#include "inputhook.h"

#include "landscape/terrain.h"
#include "gui/minimapview.h"

class Resource;
class BehaviorMSGQueue;
class CommandsPanel;

class PrototypesLoader;
class ProtoObject;
class GameObject;
class Game;
class Button;

class UnitList;
class World;

class EditTerrainPanel;

namespace Tempest{
  class Scene;
  }

class InGameControls : public Tempest::Widget {
  public:
    InGameControls( Resource & res,
                    BehaviorMSGQueue & q,
                    PrototypesLoader & prototypes,
                    Game &game );
    ~InGameControls();

    Tempest::signal< Tempest::Painter&, int , int> paintObjectsHud;
    Tempest::signal<const ProtoObject&, int> addObject;
    Tempest::signal<const Terrain::EditMode&> toogleEditLandMode;

    Tempest::signal<> save, load;
    Tempest::signal<> updateView;
    Tempest::signal<GameObject&>  setCameraPos;
    Tempest::signal< float, float,
                      Tempest::MouseEvent::MouseButton,
                      MiniMapView::Mode > minimapEvent;

    Tempest::signal< const Scene &,
                      ParticleSystemEngine &,
                      Tempest::Texture2d & > renderScene;

    //Tempest::Rect selection;

    void updateSelectUnits( const std::vector<GameObject*> &u );
    void onUnitDied(GameObject &obj);

    bool instalHook(InputHookBase *h);
    void removeHook(InputHookBase *h);
    void enableHooks(bool e);

    void updateValues();
    void renderMinimap( World& w );

    bool minimapMouseEvent(float x, float y, Tempest::Event::MouseButton btn , MiniMapView::Mode m);
  protected:
    void paintEvent(Tempest::PaintEvent &p);

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void mouseWheelEvent(Tempest::MouseEvent &e);

    void keyDownEvent(Tempest::KeyEvent &e);
    void keyUpEvent(Tempest::KeyEvent &e);

  private:
    Widget* createConsole( BehaviorMSGQueue & q );
    Widget* createEditPanel();
    EditTerrainPanel* createLandEdit();

    Resource & res;
    Game &game;
    PrototypesLoader & prototypes;

    void toogleEditPanel();

    Tempest::Shortcut showEditPanel;
    Widget        *editPanel;
    CommandsPanel *commands;
    UnitList      *units;
    Button *gold, *lim;
    MiniMapView * minimap;

    //Tempest::Bind::UserTexture frame;
    int currPl;

    void setCurrPl( size_t i );

    std::vector< InputHookBase* > hooks;
    struct AddUnitButton;

    void showMenu();
    void showFormBuilder();
    void addEditorObject( const ProtoObject& );

    bool isHooksEnabled;
    void removeAllHooks();

    template< class E >
    bool hookCall( void (InputHookBase::*f)(E &), E & e ){
      if( e.isAccepted() )
        return 1;

      for( size_t i=0; i<hooks.size(); ++i ){
        InputHookBase &b = *hooks[hooks.size()-i-1];

        e.accept();
        (b.*f)(e);

        if( e.isAccepted() )
          return 1;
        }

      return 0;
      }
  };

#endif // INGAMECONTROLS_H
