#include "panel.h"

#include "resource.h"

Panel::Panel( Resource & res ) {
  back .data = res.pixmap("gui/background");
  frame.data = res.pixmap("gui/frame2");

  resize(200, 400);
  mouseTracking = false;
  dragable      = false;

  setLayout( MyWidget::Horizontal );
  }

void Panel::setDragable(bool d) {
  dragable = d;
  }

bool Panel::isDragable() {
  return dragable;
  }

void Panel::mouseDownEvent(MyWidget::MouseEvent &e) {/*
  e.ignore();
  Widget::mousePressEvent(e);
  if( e.isAccepted() )
    return;

  e.accept();*/

  if( !dragable ){
    return;
    }

  if( e.button==MyWidget::MouseEvent::ButtonLeft ){
    mouseTracking = true;
    oldPos = pos();
    mpos   = mapToRoot( e.pos() );
    }
  }

void Panel::mouseDragEvent(MyWidget::MouseEvent &e) {
  if( !dragable ){
    e.ignore();
    return;
    }

  if( mouseTracking )
    setPosition( oldPos - (mpos - mapToRoot(e.pos() )) );
  }

void Panel::mouseMoveEvent(MyWidget::MouseEvent &e) {
  if( !dragable ){
    e.ignore();
    }
  }

void Panel::mouseUpEvent(MyWidget::MouseEvent &e) {
  mouseTracking = false;
  }

void Panel::mouseWheelEvent(MyWidget::MouseEvent &e) {

  }

void Panel::paintEvent( MyWidget::PaintEvent &e ) {
  MyWidget::Painter p(e);

  p.setTexture( back );
  p.drawRect( 0,0, w(), h() );

  p.setTexture( frame );

  p.drawRect( 20, 0, w()-40, 20,
              20, 0, 10, 20 );

  p.drawRect( 20, h()-20, w()-40, 20,
              20, 30, 10, 20 );

  p.drawRect( 0, 20, 20, h()-40,
              0, 20, 20, 10 );

  p.drawRect( w()-20, 20, 20, h()-40,
              30, 20, 20, 10 );


  p.drawRect(      0, 0, 20, 20 );
  p.drawRect( w()-20, 0, 20, 20,
              30, 0 );

  p.drawRect(      0, h()-20, 20, 20,
                   0, 30 );
  p.drawRect( w()-20, h()-20, 20, 20,
              30, 30 );

  p.unsetTexture();

  paintNested(e);
  }
