#include "ui_unitinfo.h"

#include <MyWidget/Layout>
#include "gui/button.h"
#include "gui/panel.h"
#include "gui/scroolwidget.h"
#include "gui/lineedit.h"

using namespace MyWidget;

void UI::UnitInfo::setupUi( Widget* w, Resource & res ) {
  w->setSizePolicy( Expanding, Expanding );
  w->setMaximumSize( 65536, 65536 );
  w->setMinimumSize( 20, 20 );
  w->setLayout( Horizontal );
  { Widget* owner = w;
    w = new MyWidget::Widget();
    w->setSizePolicy( Preferred, Preferred );
    w->setMaximumSize( 65536, 65536 );
    w->setMinimumSize( 20, 20 );
    w->setLayout( Vertical );
    { Widget* owner = w;
      w = new MyWidget::Widget();
      priview = w;
      w->setSizePolicy( Preferred, Preferred );
      w->setMaximumSize( 65536, 65536 );
      w->setMinimumSize( 20, 20 );
      w->setLayout( Horizontal );
      owner->layout().add(w);
      w = owner;
      } 
    { Widget* owner = w;
      w = new Panel(res);
      hpBox = (Panel*)w;
      w->setSizePolicy( Preferred, Preferred );
      w->setMaximumSize( 65536, 65536 );
      w->setMinimumSize( 20, 20 );
      w->setLayout( Horizontal );
      { Widget* owner = w;
        w = new LineEdit(res);
        hp = (LineEdit*)w;
        ((LineEdit*)w)->setText( L"\x31\x30\x30\x2f\x31\x30\x30" ); 
        w->setSizePolicy( Preferred, FixedMax );
        w->setMaximumSize( 65536, 24 );
        w->setMinimumSize( 20, 24 );
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
    w->setSizePolicy( Preferred, Preferred );
    w->setMaximumSize( 65536, 65536 );
    w->setMinimumSize( 20, 20 );
    w->setLayout( Vertical );
    { Widget* owner = w;
      w = new LineEdit(res);
      caption = (LineEdit*)w;
      ((LineEdit*)w)->setText( L"\x55\x6e\x69\x74\x20\x4e\x61\x6d\x65" ); 
      w->setSizePolicy( Preferred, FixedMax );
      w->setMaximumSize( 65536, 24 );
      w->setMinimumSize( 20, 24 );
      w->setLayout( Horizontal );
      owner->layout().add(w);
      w = owner;
      } 
    { Widget* owner = w;
      w = new MyWidget::Widget();
      status = w;
      w->setSizePolicy( Preferred, Preferred );
      w->setMaximumSize( 65536, 65536 );
      w->setMinimumSize( 20, 20 );
      w->setLayout( Horizontal );
      owner->layout().add(w);
      w = owner;
      } 
    owner->layout().add(w);
    w = owner;
    } 
  }

