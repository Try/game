#ifndef INGAMECONTROLS_H
#define INGAMECONTROLS_H

#include <MyWidget/Widget>
#include "graphics/paintergui.h"
#include "inputhook.h"

class Resource;
class BehaviorMSGQueue;
class CommandsPanel;

class PrototypesLoader;
class ProtoObject;
class GameObject;
class Game;
class Button;

class UnitList;

class InGameControls : public MyWidget::Widget {
  public:
    InGameControls( Resource & res,
                    BehaviorMSGQueue & q,
                    PrototypesLoader & prototypes,
                    Game &game );

    MyWidget::signal< MyWidget::Painter&, int , int> paintObjectsHud;
    MyWidget::signal<const ProtoObject&, int> addObject;
    MyWidget::signal<> toogleEditLandMode;

    MyWidget::signal<> save, load;

    MyWidget::Rect selection;

    void updateSelectUnits( const std::vector<GameObject*> &u );
    void onUnitDied(GameObject &obj);

    bool instalHook(InputHookBase *h);
    void removeHook(InputHookBase *h);
    void enableHooks(bool e);

  protected:
    void paintEvent(MyWidget::PaintEvent &p);

    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);
    void mouseMoveEvent(MyWidget::MouseEvent &e);

    void mouseWheelEvent(MyWidget::MouseEvent &e);

    void keyDownEvent(MyWidget::KeyEvent &e);
    void keyUpEvent(MyWidget::KeyEvent &e);

  private:
    Widget* createConsole( BehaviorMSGQueue & q );
    Widget* createEditPanel();

    Resource & res;
    Game &game;
    PrototypesLoader & prototypes;

    CommandsPanel *commands;
    UnitList      *units;
    Button *gold, *lim;
    MyWidget::Bind::UserTexture frame;
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
