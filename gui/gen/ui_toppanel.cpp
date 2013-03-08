#include "ui_toppanel.h"

#include <Tempest/Layout>
#include "gui/button.h"
#include "gui/panel.h"

using namespace Tempest;

void UI::TopPanel::setupUi( Widget* w, Resource & res ) {
  w->setSizePolicy( Expanding, Expanding );
  w->setMaximumSize( 65536, 65536 );
  w->setMinimumSize( 20, 20 );
  w->setLayout( Horizontal );
  { Widget* owner = w;
    w = new Widget();
    w->setSizePolicy( Preferred, Preferred );
    w->setMaximumSize( 65536, 65536 );
    w->setMinimumSize( 20, 20 );
    w->setLayout( Horizontal );
    { Widget* owner = w;
      w = new Button(res);
      fullScr = (Button*)w;
      ((Button*)w)->setText( L"\x46\x75\x6c\x6c\x20\x53\x63\x72\x65\x65\x6e" );
      w->setSizePolicy( FixedMax, FixedMax );
      w->setMaximumSize( 128, 27 );
      w->setMinimumSize( 27, 27 );
      w->setLayout( Horizontal );
      owner->layout().add(w);
      w = owner;
      }
    { Widget* owner = w;
      w = new Button(res);
      frmEdit = (Button*)w;
      ((Button*)w)->setText( L"\x46\x6f\x72\x6d\x20\x45\x64\x69\x74\x6f\x72" );
      w->setSizePolicy( FixedMax, FixedMax );
      w->setMaximumSize( 128, 27 );
      w->setMinimumSize( 27, 27 );
      w->setLayout( Horizontal );
      owner->layout().add(w);
      w = owner;
      }
    { Widget* owner = w;
      w = new Widget();
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
  { Widget* owner = w;
    w = new Button(res);
    menu = (Button*)w;
    ((Button*)w)->setText( L"\x41c\x435\x43d\x44e" );
    w->setSizePolicy( FixedMax, FixedMax );
    w->setMaximumSize( 128, 27 );
    w->setMinimumSize( 27, 27 );
    w->setLayout( Vertical );
    owner->layout().add(w);
    w = owner;
    }
  { Widget* owner = w;
    w = new Widget();
    w->setSizePolicy( Preferred, Preferred );
    w->setMaximumSize( 65536, 65536 );
    w->setMinimumSize( 20, 20 );
    w->setLayout( Horizontal );
    { Widget* owner = w;
      w = new Widget();
      w->setSizePolicy( Preferred, Preferred );
      w->setMaximumSize( 65536, 65536 );
      w->setMinimumSize( 20, 20 );
      w->setLayout( Horizontal );
      owner->layout().add(w);
      w = owner;
      }
    { Widget* owner = w;
      w = new Button(res);
      gold = (Button*)w;
      ((Button*)w)->setText( L"\x67\x6f\x6c\x64" );
      w->setSizePolicy( FixedMax, FixedMax );
      w->setMaximumSize( 128, 27 );
      w->setMinimumSize( 27, 27 );
      w->setLayout( Horizontal );
      owner->layout().add(w);
      w = owner;
      }
    { Widget* owner = w;
      w = new Button(res);
      lim = (Button*)w;
      ((Button*)w)->setText( L"\x6c\x69\x6d" );
      w->setSizePolicy( FixedMax, FixedMax );
      w->setMaximumSize( 128, 27 );
      w->setMinimumSize( 27, 27 );
      w->setLayout( Horizontal );
      owner->layout().add(w);
      w = owner;
      }
    owner->layout().add(w);
    w = owner;
    }
  }



