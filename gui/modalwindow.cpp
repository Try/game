#include "modalwindow.h"

#include "overlaywidget.h"
#include "centralwidget.h"

#include "resource.h"

ModalWindow::ModalWindow( Resource &res, Widget *owner ) {
  frame.data = res.pixmap("gui/colors");

  CentralWidget* r = (CentralWidget*)owner->findRoot();
/*
  while( r->layout().widgets().size()>0 ){
    r = r->layout().widgets().back();
    }
*/
  OverlayWidget *w = new OverlayWidget();
  w->setLayout( MyWidget::Horizontal );
  r->layout().add(w);

  w->setPosition(0,0);

  w->layout().add( this );
  setFocus(1);
  }

ModalWindow::~ModalWindow() {
  onClosed();
  }

void ModalWindow::mouseDownEvent(MyWidget::MouseEvent &e)
{
}

void ModalWindow::mouseWheelEvent(MyWidget::MouseEvent &)
{
}

void ModalWindow::mouseMoveEvent(MyWidget::MouseEvent &e)
{
}

void ModalWindow::paintEvent(MyWidget::PaintEvent &e) {
  {
  MyWidget::Painter p(e);
  p.setTexture( frame );
  p.setBlendMode( MyWidget::alphaBlend );
  p.drawRect( 0,0, w(), h(),
              0,4, 1,1 );
  }

  paintNested(e);
  }
