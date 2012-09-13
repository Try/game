#ifndef MAINGUI_H
#define MAINGUI_H

#include "graphics/paintergui.h"

#include <MyWidget/Widget>
#include <MyWidget/Image>

#include "resource.h"

#include "gui/panel.h"
#include "gui/button.h"

#include "gui/inputhook.h"

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

class MainGui {
  public:
    MainGui( MyGL::Device &dev,
             int w, int h,
             Resource &r,
             PrototypesLoader & prototypes );
    ~MainGui();

    void createControls(BehaviorMSGQueue &msg);
    MyWidget::signal<const ProtoObject&> addObject;
    MyWidget::signal< MyWidget::Painter&, int, int> paintObjectsHud;
    MyWidget::signal<> toogleFullScreen;

    bool draw( GUIPass & pass );
    void resizeEvent( int w, int h );

    int mouseDownEvent  (MyWidget::MouseEvent &e);
    int mouseUpEvent    (MyWidget::MouseEvent &e);
    int mouseMoveEvent  (MyWidget::MouseEvent &e);
    int mouseWheelEvent (MyWidget::MouseEvent &e);

    int keyDownEvent( MyWidget::KeyEvent & e );
    int keyUpEvent  ( MyWidget::KeyEvent & e );

    MyWidget::Rect& selectionRect();
    void update();

    void updateSelectUnits( const std::vector<GameObject*>& u );
    bool instalHook( InputHookBase* h );
    void removeHook( InputHookBase* h );

    void enableHooks( bool e );

  private:
    typedef MyWidget::PainterDevice Painter;
    typedef MyWidget::Widget  Widget;
    typedef MyWidget::Image<> Image;
    typedef MyWidget::SizePolicy        SizePolicy;

    struct MainWidget: public Widget {
      MyWidget::signal< MyWidget::Painter&, int , int> paintObjectsHud;
      void paintEvent(MyWidget::PaintEvent &p);

      MyWidget::Rect selection;
      MyWidget::Bind::UserTexture frame;

      std::vector< InputHookBase* > hooks;
      };

    struct AddUnitButton: public Button{
      AddUnitButton( Resource & res, ProtoObject& obj );
      void click();

      MyWidget::signal<const ProtoObject&> clickedEx;
      ProtoObject& prototype;
      };

    void removeAllHooks();

    template< class E >
    bool hookCall( void (InputHookBase::*f)(E &), E & e ){
      if( e.isAccepted() )
        return 1;

      for( size_t i=0; i<widget.hooks.size(); ++i ){
        InputHookBase &b = *widget.hooks[widget.hooks.size()-i-1];

        e.accept();
        (b.*f)(e);

        if( e.isAccepted() )
          return 1;
        }

      return 0;
      }

    MainWidget widget;
    Resource & res;
    PrototypesLoader & prototypes;
    bool isHooksEnabled;

    Widget *createConsole(BehaviorMSGQueue &q);
    Widget *createEditPanel();

    CommandsPanel * commands;
  };

#endif // MAINGUI_H
