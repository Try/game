#include "closedialog.h"

#include <Tempest/Layout>
#include <Tempest/Application>

#include "panel.h"
#include "richtext.h"
#include "button.h"

#include "lang/lang.h"

bool CloseDialog::shown = false;

CloseDialog::CloseDialog(Resource & res , Widget *owner):ModalWindow(res, owner) {
  shown = true;

  Panel *p = new Panel(res);
  p->setMargin(8);
  p->setSizePolicy( Tempest::FixedMin );
  p->setMinimumSize(300, 200);
  p->setLayout( Tempest::Vertical );

  RichText* r = new RichText(res);
  r->setText( Lang::tr("$(game_menu/close_game_rq)") );
  p->layout().add( r );

  Widget* w = new Widget();
  w->setLayout( Tempest::Horizontal );
  Button *y = new Button(res),
         *n = new Button(res);
  y->setText( Lang::tr("$(game_menu/yes)") );
  n->setText( Lang::tr("$(game_menu/no)") );

  y->clicked.bind( Tempest::Application::exit );
  n->clicked.bind( this, &Widget::deleteLater );

  w->layout().add( y );
  w->layout().add( n );
  p->layout().add( w );

  setLayout( Tempest::Horizontal );
  layout().add( new Widget() );
  layout().add( p );
  layout().add( new Widget() );
  }

CloseDialog::~CloseDialog() {
  shown = false;
  }

void CloseDialog::closeEvent(Tempest::CloseEvent &) {
  deleteLater();
  }

void CloseDialog::showCloseDialog( Tempest::CloseEvent &e,
                                   Resource & res, Widget* owner ) {
  if( shown ){
    return;
    }

  new CloseDialog(res, owner);
  e.accept();
  }
