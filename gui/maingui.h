#ifndef MAINGUI_H
#define MAINGUI_H

#include "graphics/paintergui.h"
#include "landscape/terrain.h"

#include <Tempest/Surface>
#include <Tempest/Image>

#include "resource.h"

#include "gui/panel.h"
#include "gui/button.h"

#include "gui/centralwidget.h"
#include "gui/inputhook.h"

#include "gui/font.h"
#include "gui/graphicssettingswidget.h"

#include <memory>

namespace Tempest{
  class Device;
  }

class GUIPass;

class PrototypesLoader;
class ProtoObject;
class CommandsPanel;
class GameObject;
class World;
class BehaviorMSGQueue;
class OverlayWidget;
class Game;

class InGameControls;

namespace Tempest{
  class Scene;
  }

class MainGui {
  public:
    MainGui( Tempest::Device &dev,
             int w, int h,
             Resource &r,
             PrototypesLoader & prototypes );
    ~MainGui();

    void setFocus();
    void setupSelUnitsList( const World& obj );
    void onUnitDied( GameObject & obj );

    void createControls( BehaviorMSGQueue &msg,
                         Game &game );
    Tempest::signal<const ProtoObject&, int> addObject;
    Tempest::signal< int > onSetPlayer, toogleEditTab;

    Tempest::signal<const GraphicsSettingsWidget::Settings&> onSettingsChanged;

    Tempest::signal< Tempest::Painter&, int, int> paintObjectsHud;
    Tempest::signal<> toogleFullScreen;
    Tempest::signal<const Terrain::EditMode&> toogleEditLandMode;
    //Tempest::signal<> updateView;
    Tempest::signal<GameObject&> setCameraPos;
    Tempest::signal< float, float,
                      Tempest::MouseEvent::MouseButton,
                      MiniMapView::Mode> minimapEvent;

    Tempest::signal< const std::wstring& > save, load;    

    Tempest::signal< const Scene &,
                      ParticleSystemEngine &,
                      Tempest::Texture2d & > renderScene;

    Tempest::signal<> updateView;

    void resizeEvent( int w, int h );

    int mouseDownEvent  (Tempest::MouseEvent &e);
    int mouseUpEvent    (Tempest::MouseEvent &e);
    int mouseMoveEvent  (Tempest::MouseEvent &e);
    int mouseWheelEvent (Tempest::MouseEvent &e);

    int scutEvent   ( Tempest::KeyEvent & e );
    int keyDownEvent( Tempest::KeyEvent & e );
    int keyUpEvent  ( Tempest::KeyEvent & e );
    bool minimapMouseEvent(float x, float y, Tempest::Event::MouseButton btn , MiniMapView::Mode m);

    Tempest::Rect& selectionRect();
    void update();

    void updateSelectUnits(const std::vector<GameObject *> &u );
    bool instalHook( InputHookBase* h );
    void removeHook( InputHookBase* h );

    void enableHooks( bool e );

    void setupMinimap(World *w );
    void renderMinimap();
    void updateValues();

    bool isCutsceneMode();

    static void drawFrame(Tempest::Painter & p,
                           const Tempest::Sprite &tex ,
                           const Tempest::Point &pos,
                           const Tempest::Size &size );

    Tempest::Surface* centralWidget();

    void setCutsceneMode( bool cs );
    void setFPS( float f );

    Resource & res;

    void buildVBO( Tempest::SurfaceRender & r0,
                   Tempest::SurfaceRender & ui,
                   Tempest::SurfaceRender & hint,
                   Tempest::LocalVertexBufferHolder &lvbo,
                   Tempest::LocalIndexBufferHolder  &libo);
  private:
    typedef Tempest::PainterDevice Painter;
    typedef Tempest::Widget  Widget;
    typedef Tempest::Image<> Image;
    typedef Tempest::SizePolicy        SizePolicy;

    InGameControls *mainwidget;
    float fps;
    std::wstring fpsStr;

    CentralWidget central;
    PrototypesLoader & prototypes;

    void saveGame();
    void loadGame();

    Tempest::Point mousePos;

    Tempest::Sprite frame, hintFrame;
    Tempest::Rect selRect;

    void paintHUD( Tempest::PaintEvent &e );
    void paintHint( Tempest::PaintEvent &e );

  };

#endif // MAINGUI_H
