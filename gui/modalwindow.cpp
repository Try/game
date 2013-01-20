#include "modalwindow.h"

#include "overlaywidget.h"
#include "centralwidget.h"

ModalWindow::ModalWindow(Resource &res, Widget *owner) {
  CentralWidget* r = (CentralWidget*)owner->findRoot();
/*
  while( r->layout().widgets().size()>0 ){
    r = r->layout().widgets().back();
    }
*/
  OverlayWidget *w = new OverlayWidget(res);
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
