#include "panel.h"

#include "resource.h"

Panel::Panel( Resource &res ) {
  back .data = res.pixmap("gui/background");
  frame.data = res.pixmap("gui/frame2");

  resize(200, 400);
  mouseTracking = false;
  dragable      = false;

  setLayout( Tempest::Horizontal );
  }

void Panel::setDragable(bool d) {
  dragable = d;
  }

bool Panel::isDragable() {
  return dragable;
  }

void Panel::mouseDownEvent(Tempest::MouseEvent &e) {/*
  e.ignore();
  Widget::mousePressEvent(e);
  if( e.isAccepted() )
    return;

  e.accept();*/

  if( !dragable ){
    return;
    }

  if( e.button==Tempest::MouseEvent::ButtonLeft ){
    mouseTracking = true;
    oldPos = pos();
    mpos   = mapToRoot( e.pos() );
    }
  }

void Panel::mouseDragEvent(Tempest::MouseEvent &e) {
  if( !dragable ){
    e.ignore();
    return;
    }

  if( mouseTracking )
    setPosition( oldPos - (mpos - mapToRoot(e.pos() )) );
  }

void Panel::mouseMoveEvent(Tempest::MouseEvent &) {
  if( !dragable ){
    //e.ignore();
    }
  }

void Panel::mouseUpEvent(Tempest::MouseEvent &) {
  mouseTracking = false;
  }

void Panel::mouseWheelEvent(Tempest::MouseEvent &) {

  }

void Panel::paintEvent( Tempest::PaintEvent &e ) {
  Tempest::Painter p(e);

  if( 1 || hasFocus() ){
    p.setTexture( frame );

    p.drawRect( 20, 0, w()-40, 20,
                20, 0, 10, 20 );

    p.drawRect( 20, h()-20, w()-40, 20,
                20, 30, 10, 20 );

    p.drawRect( 0, 20, 20, h()-40,
                0, 20, 20, 10 );

    p.drawRect( w()-20, 20, 20, h()-40,
                30, 20, 20, 10 );

    p.unsetTexture();
    }

  if( 1 || hasChildFocus() ){
    p.setTexture( frame );

    p.drawRect(      0, 0, 20, 20 );
    p.drawRect( w()-20, 0, 20, 20,
                30, 0 );

    p.drawRect(      0, h()-20, 20, 20,
                     0, 30 );
    p.drawRect( w()-20, h()-20, 20, 20,
                30, 30 );

    p.unsetTexture();
    }

  int sz = 8;
  p.setTexture( back );
  p.drawRect( sz,sz-1, w()-2*sz, h()-2*sz+2 );

  paintNested(e);
  }
