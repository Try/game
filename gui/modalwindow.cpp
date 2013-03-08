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
  w->setLayout( Tempest::Horizontal );
  r->layout().add(w);

  w->setPosition(0,0);

  w->layout().add( this );
  setFocus(1);
  }

ModalWindow::~ModalWindow() {
  onClosed();
  }

void ModalWindow::mouseDownEvent(Tempest::MouseEvent &e)
{
}

void ModalWindow::mouseWheelEvent(Tempest::MouseEvent &)
{
}

void ModalWindow::mouseMoveEvent(Tempest::MouseEvent &e)
{
}

void ModalWindow::paintEvent(Tempest::PaintEvent &e) {
  {
  Tempest::Painter p(e);
  p.setTexture( frame );
  p.setBlendMode( Tempest::alphaBlend );
  p.drawRect( 0,0, w(), h(),
              0,4, 1,1 );
  }

  paintNested(e);
  }

void ModalWindow::shortcutEvent(Tempest::KeyEvent &e) {
  Widget::shortcutEvent(e);
  e.accept();
  }
