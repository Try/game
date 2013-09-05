#include "unitlist.h"

#include "gui/button.h"
#include <Tempest/Layout>

#include "resource.h"
#include "game/gameobject.h"

#include "unitinfo.h"

#include "commandspanel.h"
#include "unitview.h"

#include "game.h"
#include "util/weakworldptr.h"

#include <Tempest/Painter>

struct UnitList::Btn : public Button {
  Btn( Resource & res ):Button(res){
    Button::clicked.bind( this, &Btn::onClick );
    oldHp = 255;
    }

  void onClick(){
    clicked(owner);
    }

  void paintEvent(Tempest::PaintEvent &e){
    Button::paintEvent(e);

    Tempest::Painter p(e);

    //PainterGUI& pt = (PainterGUI&)p.device();
    p.setColor( 1, 1-std::max(1-oldHp/200.0, 0.0), 1-std::max(1-oldHp/128.0, 0.0), 1 );

    p.setBlendMode( Tempest::alphaBlend );
    p.setTexture( icon );

    int sz = std::min(w(), h());
    float k = std::min( sz/float(icon.width()),
                        sz/float(icon.height()) );

    int icW = icon.width() *k,
        icH = icon.height()*k;

    Tempest::Rect s = p.scissor();
    p.setScissor( s.intersected(viewRect()) );
    p.drawRect( (w()-icW)/2, (h()-icH)/2, icW, icH,
                0, 0, icon.width(), icon.height() );
    p.setScissor(s);

    drawFrame(p);
    }

  void customEvent(Tempest::CustomEvent &){
    if( owner==0 )
      return;

    int nHp = (255*owner->hp())/std::max(1, owner->getClass().data.maxHp);
    if( nHp!=oldHp ){
      oldHp = nHp;
      update();
      }
    }

  GameObject * owner;
  Tempest::signal<GameObject*> clicked;

  Tempest::Sprite icon;
  int oldHp;
  };

struct UnitList::Lay : public Tempest::Layout {
  void applyLayout(){
    }
  };

struct UnitList::View: public Tempest::Widget{
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
  view->setSizePolicy( Tempest::FixedMin, Tempest::FixedMin );

  centralWidget().layout().add( view );

  setMinimumSize( sz*11, sz*3 );

  setSizePolicy( Tempest::FixedMin, Tempest::Preferred );
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
    b->clicked.bind( this, &UnitList::onBtn );
    b->icon = res.pixmap("gui/icon/"+units[i]->getClass().name);
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
          uinfo->setup( btn[r]->owner );
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
