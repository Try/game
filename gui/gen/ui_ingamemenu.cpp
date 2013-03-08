#include "ui_ingamemenu.h"

#include <Tempest/Layout>
#include "gui/button.h"
#include "gui/panel.h"
#include "gui/scroolwidget.h"

using namespace Tempest;

void UI::InGameMenu::setupUi( Widget* w, Resource & res ) {
  w->setSizePolicy( Expanding, Expanding );
  w->setMaximumSize( 65536, 65536 );
  w->setMinimumSize( 20, 20 );
  w->setLayout( Vertical );
  { Widget* owner = w;
    w = new Button(res);
    resume = (Button*)w;
    ((Button*)w)->setText( L"\x41f\x440\x43e\x434\x43e\x43b\x436\x438\x442\x44c" );
    w->setSizePolicy( FixedMax, FixedMax );
    w->setMaximumSize( 128, 27 );
    w->setMinimumSize( 27, 27 );
    w->setLayout( Vertical );
    owner->layout().add(w);
    w = owner;
    }
  { Widget* owner = w;
    w = new Button(res);
    save = (Button*)w;
    ((Button*)w)->setText( L"\x421\x43e\x445\x440\x430\x43d\x438\x442\x44c" );
    w->setSizePolicy( FixedMax, FixedMax );
    w->setMaximumSize( 128, 27 );
    w->setMinimumSize( 27, 27 );
    w->setLayout( Horizontal );
    owner->layout().add(w);
    w = owner;
    }
  { Widget* owner = w;
    w = new Button(res);
    load = (Button*)w;
    ((Button*)w)->setText( L"\x417\x430\x433\x440\x443\x437\x438\x442\x44c" );
    w->setSizePolicy( FixedMax, FixedMax );
    w->setMaximumSize( 128, 27 );
    w->setMinimumSize( 27, 27 );
    w->setLayout( Horizontal );
    owner->layout().add(w);
    w = owner;
    }
  { Widget* owner = w;
    w = new Button(res);
    quit = (Button*)w;
    ((Button*)w)->setText( L"\x41f\x43e\x43a\x438\x43d\x443\x442\x44c\x20\x438\x433\x440\x443" );
    w->setSizePolicy( FixedMax, FixedMax );
    w->setMaximumSize( 128, 27 );
    w->setMinimumSize( 27, 27 );
    w->setLayout( Horizontal );
    owner->layout().add(w);
    w = owner;
    }
  }

