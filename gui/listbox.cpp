#include "listbox.h"

#include "gui/maingui.h"
#include "gui/overlaywidget.h"
#include "gui/button.h"
#include "gui/scroolwidget.h"
#include "gui/panel.h"

ListBox::ListBox(MainGui &gui, Resource &r) : Button(r), gui(gui), res(r) {
  clicked.bind( *this, &ListBox::showList );
  }

void ListBox::showList() {
  MyWidget::Widget *w = gui.addOverlay();
  w->setPosition( mapToRoot( MyWidget::Point(0,h()) ) );

  Panel *box = new Panel(res);
  box->setLayout( MyWidget::Horizontal );
  box->layout().setMargin(6);

  ScroolWidget *sw = new ScroolWidget( res );
  for( int i=0; i<10; ++i ){
    Button * b = new ListBox(gui, res);
    //b->clicked.bind( *this, &ListBox::showList );

    sw->centralWidget().layout().add( b );
    }

  box->layout().add(sw);

  w->layout().add( box );
  w->setLayout( MyWidget::Horizontal );
  w->resize(170, 200);
  //w->layout().add( new Button(res) );
  }
