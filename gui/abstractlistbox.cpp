#include "abstractlistbox.h"

#include "gui/maingui.h"
#include "gui/overlaywidget.h"
#include "gui/button.h"
#include "gui/scroolwidget.h"
#include "gui/panel.h"

#include <Tempest/Application>

AbstractListBox::AbstractListBox( Resource &r) : Button(r), res(r) {
  clicked.bind( *this, &AbstractListBox::showList );
  overlay = 0;
  needToShow = 1;

  lastRM = Tempest::Application::tickCount();
  }

AbstractListBox::~AbstractListBox() {
  close();
  }

void AbstractListBox::showList() {
  if( !needToShow )
    return;

  Widget* r = findRoot();

  while( r->layout().widgets().size()>1 ){
    r = r->layout().widgets().back();
    }

  OverlayWidget *w = new OverlayWidget();
  overlay = w;
  overlay->onDestroy.bind( *this, &AbstractListBox::rmOverlay );
  r->layout().add(w);

  w->setPosition(0,0);

  Widget *box = createDropList();
  Widget *wb  = new Widget();

  if( box->w() > r->w() )
    box->resize( r->w(), box->h() );

  if( box->h() > r->h() )
    box->resize( box->w(), r->h() );

  if( box->y()+box->h() > r->h() ){
    box->setPosition( box->x(), r->h()-box->h() );
    }

  if( box->x()+box->w() > r->w() ){
    box->setPosition( r->w()-box->w(), box->y() );
    }

  wb->layout().add( box );
  w->layout().add(wb);
  box->setFocus(1);

  w->setupSignals();
  }

void AbstractListBox::close() {
  if( overlay ){
    overlay->onDestroy.ubind( *this, &AbstractListBox::rmOverlay );
    overlay->deleteLater();
    overlay = 0;
    }
  }

void AbstractListBox::rmOverlay(Tempest::Widget *) {
  overlay = 0;
  lastRM = Tempest::Application::tickCount();
  }

void AbstractListBox::mouseDownEvent(Tempest::MouseEvent &e) {
  needToShow = Tempest::Application::tickCount()!=lastRM;
  Button::mouseDownEvent(e);
  }

void AbstractListBox::mouseUpEvent(Tempest::MouseEvent &e) {
  Button::mouseUpEvent(e);
  needToShow = true;
  }

Tempest::Widget* AbstractListBox::createDropList() {
  Panel *box = new Panel(res);
  box->setLayout( Tempest::Horizontal );
  box->layout().setMargin(6);
  box->setPosition( mapToRoot( Tempest::Point(0,h()) ) );
  box->resize(170*MainGui::uiScale, 200*MainGui::uiScale);

  ScroolWidget *sw = new ScroolWidget( res );
  for( int i=0; i<10; ++i ){
    Button * b = new AbstractListBox(res);
    //b->clicked.bind( *this, &AbstractListBox::showList );

    sw->centralWidget().layout().add( b );
    }

  box->layout().add(sw);
  return box;
  }
