#ifndef INPUTHOOK_H
#define INPUTHOOK_H

#include <Tempest/Event>
#include <Tempest/signal>

#include "gui/minimapview.h"

class InputHookBase {
  public:
    virtual ~InputHookBase(){}

    Tempest::signal<> onRemove;

    virtual void mouseDownEvent ( Tempest::MouseEvent & e ){ e.ignore(); }
    virtual void mouseUpEvent   ( Tempest::MouseEvent & e ){ e.ignore();}

    virtual void mouseMoveEvent ( Tempest::MouseEvent & e ){ e.ignore(); }

    virtual void mouseWheelEvent ( Tempest::MouseEvent & e ){ e.ignore(); }

    virtual void keyDownEvent( Tempest::KeyEvent & e ) { e.ignore(); }
    virtual void keyUpEvent  ( Tempest::KeyEvent & e ) { e.ignore(); }

    virtual bool minimapMouseEvent( float /*x*/, float /*y*/,
                                    Tempest::Event::MouseButton /*btn*/,
                                    MiniMapView::Mode /*m*/ ){return 0; }
  };

struct InputHook : public InputHookBase {
  virtual void mouseDownEvent ( Tempest::MouseEvent & );
  virtual void mouseUpEvent   ( Tempest::MouseEvent & );
  virtual void mouseMoveEvent ( Tempest::MouseEvent & );

  virtual bool minimapMouseEvent(float x, float y,
                                 Tempest::Event::MouseButton btn,
                                 MiniMapView::Mode m );

  virtual void mouseWheelEvent ( Tempest::MouseEvent & );

  virtual void keyDownEvent( Tempest::KeyEvent & );
  virtual void keyUpEvent  ( Tempest::KeyEvent & );

  Tempest::signal<Tempest::MouseEvent &> mouseDown, mouseMove,
                                           mouseUp,   mouseWheel;
  Tempest::signal<Tempest::KeyEvent &>   keyDown, keyUp;
  };

#endif // INPUTHOOK_H
