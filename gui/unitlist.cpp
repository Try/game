#include "unitlist.h"

#include "gui/button.h"
#include <MyWidget/Layout>

#include "resource.h"
#include "game/gameobject.h"

#include "unitinfo.h"

#include "commandspanel.h"
#include "unitview.h"

#include "game.h"
#include "util/weakworldptr.h"

#include <MyWidget/Painter>

struct UnitList::Btn : public Button {
  Btn( Resource & res ):Button(res){
    Button::clicked.bind(*this, &Btn::onClick );
    oldHp = 255;
    }

  void onClick(){
    clicked(owner);
    }

  void paintEvent(MyWidget::PaintEvent &e){
    Button::paintEvent(e);

    MyWidget::Painter p(e);

    PainterGUI& pt = (PainterGUI&)p.device();
    pt.setColor( 1, 1-std::max(1-oldHp/200.0, 0.0), 1-std::max(1-oldHp/128.0, 0.0), 1 );

    p.setBlendMode( MyWidget::alphaBlend );
    p.setTexture( icon );
    p.drawRect( 0, 0, w(), h(),
                0, 0, icon.data.rect.w, icon.data.rect.h );
    }

  void customEvent(MyWidget::CustomEvent &){
    if( owner==0 )
      return;

    int nHp = (255*owner->hp())/std::max(1, owner->getClass().data.maxHp);
    if( nHp!=oldHp ){
      oldHp = nHp;
      update();
      }
    }

  GameObject * owner;
  MyWidget::signal<GameObject*> clicked;

  Texture icon;
  int oldHp;
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
    b->clicked.bind(*this, &UnitList::onBtn );
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

void UnitList::onBtn( GameObject * obj ) {
  if( obj==0 )
    return;

  int pl = obj->playerNum();
  size_t id = obj->world().objectWPtr(obj).id();
  obj->game().message( pl, Behavior::Select, id );
  }
