#include "listbox.h"

#include "gui/maingui.h"
#include "gui/overlaywidget.h"
#include "gui/button.h"
#include "gui/scroolwidget.h"

ListBox::ListBox(MainGui &gui, Resource &r) : Button(r), gui(gui), res(r) {
  clicked.bind( *this, &ListBox::showList );
  }

void ListBox::showList() {
  MyWidget::Widget *w = gui.addOverlay();
  w->setPosition( mapToRoot( MyWidget::Point(0,h()) ) );

  ScroolWidget *sw = new ScroolWidget( res );
  for( int i=0; i<10; ++i )
    sw->centralWidget().layout().add( new Button(res) );
  w->layout().add( sw );
  w->setLayout( MyWidget::Horizontal );
  //w->layout().add( new Button(res) );
  }
