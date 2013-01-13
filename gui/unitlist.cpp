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

UnitList::UnitList( CommandsPanel *panel,
                    Resource & res,
                    UnitView *uview,
                    UnitInfo *uinf )
  :ScroolWidget(res), res(res), uview(uview), uinfo(uinf), cmd(panel) {

  uview->setCameraPos.bind( setCameraPos );

  setScroolBarVisible(0);
  const int sz = 40;

  view = new View();
  view->setMinimumSize( sz*10+4, sz*3 );
  view->setSizePolicy( MyWidget::FixedMin, MyWidget::FixedMin );

  centralWidget().layout().add( view );

  setMinimumSize( sz*11, sz*3 );

  setSizePolicy( MyWidget::FixedMin, MyWidget::Preferred );
  }

void UnitList::setup(const std::vector<GameObject*> &ux ) {
  units = ux;

  if( units.size() ){
    cmd->bind( units[0]);
    uview->setupUnit( units[0] );
    uinfo->setup( units[0] );
    } else {
    cmd->bind(0);
    uview->setupUnit( 0 );
    uinfo->setup( 0 );
    }

  view->layout().removeAll();
  btn.clear();
  setScroolBarVisible( units.size()>30 );

  const int sz = 40;

  view->setMinimumSize( sz*10+4, sz*(units.size()/10+1) );

  for( size_t i=0; i<units.size(); ++i ){
    Btn *b = new Btn(res);
    b->icon.data = res.pixmap("gui/icon/"+units[i]->getClass().name);
    b->owner = units[i];
    int x = sz*(i%10);
    if( i%10>=5 )
      x += 4;
    b->setGeometry( x, sz*(i/10), sz, sz );

    view->layout().add( b );
    btn.push_back(b);
    }
  }

void UnitList::onUnitDied(GameObject &obj) {
  units.resize( std::remove( units.begin(), units.end(), &obj ) - units.begin() );

  if( units.size()==1 ){
    setup( units );
    return;
    }

  for( size_t i=0; i<btn.size(); ++i ){
    if( btn[i] && btn[i]->owner==&obj ){
      btn[i]->deleteLater();
      btn[i] = 0;

      for( size_t r=0; r<btn.size(); ++r )
        if( btn[r] && btn[r]->owner ){
          cmd->bind(btn[r]->owner);
          uview->setupUnit( btn[r]->owner );
          return;
          }
      }
    }

  if( units.size() )
    return;

  cmd->bind(0);
  uview->setupUnit( 0 );
  uinfo->setup( 0 );
  }
