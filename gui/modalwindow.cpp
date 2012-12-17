#include "modalwindow.h"

#include "overlaywidget.h"

ModalWindow::ModalWindow(Resource &res, Widget *owner) {
  Widget* r = owner->findRoot();

  while( r->layout().widgets().size()>1 ){
    r = r->layout().widgets().back();
    }

  OverlayWidget *w = new OverlayWidget(res);
  w->setLayout( MyWidget::Horizontal );
  r->layout().add(w);

  w->setPosition(0,0);

  w->layout().add( this );
  setFocus(1);
}

void ModalWindow::mouseWheelEvent(MyWidget::MouseEvent &)
{
}
