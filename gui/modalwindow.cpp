#include "modalwindow.h"

#include "overlaywidget.h"
#include "centralwidget.h"

#include "resource.h"

ModalWindow::ModalWindow( Resource &res, Widget *owner ) {
  frame = res.pixmap("gui/colors");

  CentralWidget* r = (CentralWidget*)owner->findRoot();
/*
  while( r->layout().widgets().size()>0 ){
    r = r->layout().widgets().back();
    }
*/
  OverlayWidget *w = new OverlayWidget();
  //w->setLayout( Tempest::Horizontal );
  r->layout().add(w);

  w->setPosition(0,0);

  setPosition(0,0);
  resize( r->size() );

  w->layout().add( this );
  setFocus(1);
  }

ModalWindow::~ModalWindow() {
  onClosed();
  }

void ModalWindow::mouseDownEvent(Tempest::MouseEvent &)
{
}

void ModalWindow::mouseWheelEvent(Tempest::MouseEvent &)
{
}

void ModalWindow::mouseMoveEvent(Tempest::MouseEvent &)
{
}

void ModalWindow::paintEvent(Tempest::PaintEvent &e) {
  {
  Tempest::Painter p(e);
  p.setBlendMode( Tempest::alphaBlend );
  p.setColor( Tempest::Color(0,0,0,0.5));
  p.drawRect( 0,0, w(), h() );
  }

  paintNested(e);
  }

void ModalWindow::shortcutEvent(Tempest::KeyEvent &e) {
  Widget::shortcutEvent(e);
  e.accept();
  }

void ModalWindow::closeEvent(Tempest::CloseEvent &e) {
#ifdef __ANDROID__
  (void)e;
  deleteLater();
#else
  e.ignore();
  //deleteLater();
#endif
  }
