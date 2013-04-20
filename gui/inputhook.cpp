#include "inputhook.h"

void InputHook::mouseDownEvent( Tempest::MouseEvent &e ) {
  mouseDown(e);
  if( mouseDown.bindsCount()==0 )
    e.ignore();
  }

void InputHook::mouseUpEvent( Tempest::MouseEvent &e ) {
  mouseUp(e);
  if( mouseUp.bindsCount()==0 )
    e.ignore();
  }

void InputHook::mouseMoveEvent( Tempest::MouseEvent &e ) {
  mouseMove(e);
  if( mouseMove.bindsCount()==0 )
    e.ignore();
  }

bool InputHook::minimapMouseEvent( float x, float y,
                                   Tempest::Event::MouseButton btn,
                                   MiniMapView::Mode m) {
  (void)x;
  (void)y;

  Tempest::MouseEvent e(-1, -1, btn);
  e.accept();

  if( m==MiniMapView::Down )
    mouseDownEvent(e); else
  if( m==MiniMapView::Drag )
    mouseMoveEvent(e); else
  if( m==MiniMapView::Up )
    mouseUpEvent(e);

  return e.isAccepted();
  }

void InputHook::mouseWheelEvent( Tempest::MouseEvent &e ) {
  mouseWheel(e);
  if( mouseWheel.bindsCount()==0 )
    e.ignore();
  }

void InputHook::keyDownEvent( Tempest::KeyEvent &e ) {
  keyDown(e);
  if( keyDown.bindsCount()==0 )
    e.ignore();
  }

void InputHook::keyUpEvent( Tempest::KeyEvent &e ) {
  keyUp(e);
  if( keyUp.bindsCount()==0 )
    e.ignore();
  }
