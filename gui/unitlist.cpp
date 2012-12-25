#include "unitlist.h"

#include "gui/button.h"
#include <MyWidget/Layout>

#include "resource.h"
#include "game/gameobject.h"

#include "unitinfo.h"

#include "commandspanel.h"
#include "unitview.h"

struct UnitList::Btn : public Button {
  Btn( Resource & res ):Button(res){}

  GameObject * owner;
  };

struct UnitList::Lay : public MyWidget::Layout {
  void applyLayout(){
    }
  };

struct UnitList::View: public MyWidget::Widget{
  View(){
    setLayout( new Lay() );
    }
  };

UnitList::UnitList(CommandsPanel *panel, Resource & res , UnitView *uview)
  :ScroolWidget(res), res(res), uview(uview), cmd(panel) {
  //setLayout( new Lay() );
  setScroolBarVisible(0);
  const int sz = 40;

  view = new View();
  view->setMinimumSize( sz*10, sz*3 );
  view->setSizePolicy( MyWidget::FixedMin, MyWidget::FixedMin );

  centralWidget().layout().add( view );

  setMinimumSize( sz*11, sz*3 );

  setSizePolicy( MyWidget::FixedMin, MyWidget::Preferred );
  }

void UnitList::setup(const std::vector<GameObject*> &u ) {
  if( u.size() ){
    cmd->bind(u[0]);
    uview->setupUnit( u[0] );
    } else {
    cmd->bind(0);
    uview->setupUnit( 0 );
    }

  view->layout().removeAll();
  btn.clear();
  setScroolBarVisible( u.size()>30 );

  const int sz = 40;

  view->setMinimumSize( sz*10, sz*(u.size()/10+1) );

  if( u.size()==1 ){
    view->setMinimumSize( sz*10, h() );
    UnitInfo *ux = new UnitInfo( *u[0], res );
    ux->resize( sz*10, view->h() );

    view->layout().add( ux );
    return;
    }

  for( size_t i=0; i<u.size(); ++i ){
    Btn *b = new Btn(res);
    b->icon.data = res.pixmap("gui/icon/"+u[i]->getClass().name);
    b->owner = u[i];
    b->setGeometry( sz*(i%10), sz*(i/10), sz, sz );

    view->layout().add( b );
    btn.push_back(b);
    }
  }

void UnitList::onUnitDied(GameObject &obj) {
  for( size_t i=0; i<btn.size(); ++i ){
    if( btn[i] && btn[i]->owner==&obj ){
      btn[i]->deleteLater();
      btn[i] = 0;

      for( size_t r=0; r<btn.size(); ++r )
        if( btn[i] ){
          cmd->bind(btn[i]->owner);
          uview->setupUnit( btn[i]->owner );
          return;
          }
      }
    }

  cmd->bind(0);
  uview->setupUnit( 0 );
  }
