#include "inputhook.h"

void InputHook::mouseDownEvent( MyWidget::MouseEvent &e ) {
  mouseDown(e);
  if( mouseDown.bindsCount()==0 )
    e.ignore();
  }

void InputHook::mouseUpEvent( MyWidget::MouseEvent &e ) {
  mouseUp(e);
  if( mouseUp.bindsCount()==0 )
    e.ignore();
  }

void InputHook::mouseMoveEvent( MyWidget::MouseEvent &e ) {
  mouseMove(e);
  if( mouseMove.bindsCount()==0 )
    e.ignore();
  }

bool InputHook::minimapMouseEvent( float x, float y,
                                   MyWidget::Event::MouseButton btn,
                                   MiniMapView::Mode m) {
  MyWidget::MouseEvent e(-1, -1, btn);
  e.accept();

  if( m==MiniMapView::Down )
    mouseDownEvent(e); else
  if( m==MiniMapView::Drag )
    mouseMoveEvent(e); else
  if( m==MiniMapView::Up )
    mouseUpEvent(e);

  return e.isAccepted();
  }

void InputHook::mouseWheelEvent( MyWidget::MouseEvent &e ) {
  mouseWheel(e);
  if( mouseWheel.bindsCount()==0 )
    e.ignore();
  }

void InputHook::keyDownEvent( MyWidget::KeyEvent &e ) {
  keyDown(e);
  if( keyDown.bindsCount()==0 )
    e.ignore();
  }

void InputHook::keyUpEvent( MyWidget::KeyEvent &e ) {
  keyUp(e);
  if( keyUp.bindsCount()==0 )
    e.ignore();
  }
