#include "ui_loaddialog.h"

#include <MyWidget/Layout>
#include "gui/button.h"
#include "gui/panel.h"
#include "gui/scroolwidget.h"
#include "gui/lineedit.h"

using namespace MyWidget;

void UI::LoadDialog::setupUi( Widget* w, Resource & res ) {
  w->setSizePolicy( Expanding, Expanding );
  w->setMaximumSize( 65536, 65536 );
  w->setMinimumSize( 20, 20 );
  w->setLayout( Vertical );
  { Widget* owner = w;
    w = new MyWidget::Widget();
    topWidget = w;
    w->setSizePolicy( Preferred, Preferred );
    w->setMaximumSize( 65536, 65536 );
    w->setMinimumSize( 20, 20 );
    w->setLayout( Horizontal );
    { Widget* owner = w;
      w = new Panel(res);
      topPanel = (Panel*)w;
      w->setSizePolicy( Preferred, Preferred );
      w->setMaximumSize( 65536, 65536 );
      w->setMinimumSize( 20, 20 );
      w->setLayout( Horizontal );
      { Widget* owner = w;
        w = new MyWidget::Widget();
        w->setSizePolicy( Preferred, Preferred );
        w->setMaximumSize( 65536, 65536 );
        w->setMinimumSize( 20, 20 );
        w->setLayout( Horizontal );
        owner->layout().add(w);
        w = owner;
        } 
      { Widget* owner = w;
        w = new LineEdit(res);
        inputName = (LineEdit*)w;
        ((LineEdit*)w)->setText( L"\x66\x6e\x61\x6d\x65" ); 
        w->setSizePolicy( Preferred, Preferred );
        w->setMaximumSize( 65536, 24 );
        w->setMinimumSize( 20, 24 );
        w->setLayout( Vertical );
        owner->layout().add(w);
        w = owner;
        } 
      { Widget* owner = w;
        w = new MyWidget::Widget();
        w->setSizePolicy( Preferred, Preferred );
        w->setMaximumSize( 65536, 65536 );
        w->setMinimumSize( 20, 20 );
        w->setLayout( Vertical );
        owner->layout().add(w);
        w = owner;
        } 
      owner->layout().add(w);
      w = owner;
      } 
    owner->layout().add(w);
    w = owner;
    } 
  { Widget* owner = w;
    w = new MyWidget::Widget();
    central = w;
    w->setSizePolicy( Preferred, Preferred );
    w->setMaximumSize( 65536, 65536 );
    w->setMinimumSize( 20, 20 );
    w->setLayout( Horizontal );
    { Widget* owner = w;
      w = new ScroolWidget(res);
      items = (ScroolWidget*)w;
      w->setSizePolicy( Preferred, Preferred );
      w->setMaximumSize( 65536, 65536 );
      w->setMinimumSize( 20, 20 );
      MyWidget::Widget *pw = w;
      w = &((ScroolWidget*)w)->centralWidget();
      w = pw;
      owner->layout().add(w);
      w = owner;
      } 
    owner->layout().add(w);
    w = owner;
    } 
  { Widget* owner = w;
    w = new MyWidget::Widget();
    bottom = w;
    w->setSizePolicy( Preferred, Preferred );
    w->setMaximumSize( 65536, 65536 );
    w->setMinimumSize( 20, 20 );
    w->setLayout( Horizontal );
    { Widget* owner = w;
      w = new Button(res);
      accept = (Button*)w;
      ((Button*)w)->setText( L"\x417\x430\x433\x440\x443\x437\x438\x442\x44c" ); 
      w->setSizePolicy( FixedMax, FixedMax );
      w->setMaximumSize( 128, 27 );
      w->setMinimumSize( 27, 27 );
      w->setLayout( Vertical );
      owner->layout().add(w);
      w = owner;
      } 
    { Widget* owner = w;
      w = new MyWidget::Widget();
      w->setSizePolicy( Preferred, Preferred );
      w->setMaximumSize( 65536, 65536 );
      w->setMinimumSize( 20, 20 );
      w->setLayout( Vertical );
      owner->layout().add(w);
      w = owner;
      } 
    { Widget* owner = w;
      w = new Button(res);
      cancel = (Button*)w;
      ((Button*)w)->setText( L"\x41e\x442\x43c\x435\x43d\x430" ); 
      w->setSizePolicy( FixedMax, FixedMax );
      w->setMaximumSize( 128, 27 );
      w->setMinimumSize( 27, 27 );
      w->setLayout( Vertical );
      owner->layout().add(w);
      w = owner;
      } 
    owner->layout().add(w);
    w = owner;
    } 
  }

