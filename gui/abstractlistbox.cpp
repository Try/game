#include "abstractlistbox.h"

#include "gui/maingui.h"
#include "gui/overlaywidget.h"
#include "gui/button.h"
#include "gui/scroolwidget.h"
#include "gui/panel.h"

AbstractListBox::AbstractListBox( Resource &r) : Button(r), res(r) {
  clicked.bind( *this, &AbstractListBox::showList );
  overlay = 0;
  }

void AbstractListBox::showList() {
  Widget* r = findRoot();

  while( r->layout().widgets().size()>1 ){
    r = r->layout().widgets().back();
    }

  OverlayWidget *w = new OverlayWidget(res);
  overlay = w;
  overlay->onDestroy.bind( *this, &AbstractListBox::rmOverlay );
  r->layout().add(w);

  w->setPosition(0,0);

  Widget *box = createDropList();
  w->layout().add( box );
  box->setFocus(1);

  w->setupSignals();
  }

void AbstractListBox::close() {
  if( overlay ){
    overlay->deleteLater();
    overlay = 0;
    }
  }

void AbstractListBox::rmOverlay(MyWidget::Widget *) {
  overlay = 0;
  }

MyWidget::Widget* AbstractListBox::createDropList() {
  Panel *box = new Panel(res);
  box->setLayout( MyWidget::Horizontal );
  box->layout().setMargin(6);
  box->setPosition( mapToRoot( MyWidget::Point(0,h()) ) );
  box->resize(170, 200);

  ScroolWidget *sw = new ScroolWidget( res );
  for( int i=0; i<10; ++i ){
    Button * b = new AbstractListBox(res);
    //b->clicked.bind( *this, &AbstractListBox::showList );

    sw->centralWidget().layout().add( b );
    }

  box->layout().add(sw);
  return box;
  }
