#ifndef MAINGUI_H
#define MAINGUI_H

#include "graphics/paintergui.h"

#include <MyWidget/Widget>
#include <MyWidget/Image>

#include "resource.h"

#include "gui/panel.h"
#include "gui/button.h"

#include "gui/centralwidget.h"
#include "gui/inputhook.h"

#include "gui/font.h"

#include <memory>

namespace MyGL{
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

class MainGui {
  public:
    MainGui( MyGL::Device &dev,
             int w, int h,
             Resource &r,
             PrototypesLoader & prototypes );
    ~MainGui();

    void setFocus();
    void setupSelUnitsList( const World& obj );
    void onUnitDied( GameObject & obj );

    void createControls( BehaviorMSGQueue &msg,
                         Game &game );
    MyWidget::signal<const ProtoObject&, int> addObject;
    MyWidget::signal< MyWidget::Painter&, int, int> paintObjectsHud;
    MyWidget::signal<> toogleFullScreen;
    MyWidget::signal<> toogleEditLandMode;

    bool draw( GUIPass & pass );
    void resizeEvent( int w, int h );

    int mouseDownEvent  (MyWidget::MouseEvent &e);
    int mouseUpEvent    (MyWidget::MouseEvent &e);
    int mouseMoveEvent  (MyWidget::MouseEvent &e);
    int mouseWheelEvent (MyWidget::MouseEvent &e);

    int scutEvent   ( MyWidget::KeyEvent & e );
    int keyDownEvent( MyWidget::KeyEvent & e );
    int keyUpEvent  ( MyWidget::KeyEvent & e );

    MyWidget::Rect& selectionRect();
    void update();

    void updateSelectUnits(const std::vector<GameObject *> &u );
    bool instalHook( InputHookBase* h );
    void removeHook( InputHookBase* h );

    void enableHooks( bool e );

  private:
    typedef MyWidget::PainterDevice Painter;
    typedef MyWidget::Widget  Widget;
    typedef MyWidget::Image<> Image;
    typedef MyWidget::SizePolicy        SizePolicy;

    InGameControls *mainwidget;

    CentralWidget central;
    Resource & res;
    PrototypesLoader & prototypes;
  };

#endif // MAINGUI_H
