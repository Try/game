#include "unitinfo.h"

#include "gen/ui_unitinfo.h"

#include "lineedit.h"
#include "button.h"
#include "progressbar.h"

#include "game/gameobject.h"
#include "behavior/recruterbehavior.h"
#include "behavior/warriorbehavior.h"

#include "maingui.h"
#include "lang/lang.h"

#include <Tempest/Layout>
#include <sstream>

#include "game.h"

struct UnitInfo::Btn : public Button{
  Btn( Resource & res ):Button(res){
    const int sz = 40;
    setMinimumSize(sz, sz);
    setMaximumSize(sz, sz);
    setSizePolicy( Tempest::FixedMax );
    }

  };

struct UnitInfo::PanelBase : public Tempest::Widget {
  PanelBase( Resource & res ): res(res) {
    frame.data = res.pixmap("gui/hintFrame");
    layout().setMargin(15);
    }

  void paintEvent(Tempest::PaintEvent &e){
    Tempest::Painter p(e);
    p.setBlendMode( Tempest::alphaBlend );
    p.setTexture( frame );

    MainGui::drawFrame(p, frame, Tempest::Point(), size() );

    paintNested(e);
    }

  Resource  & res;
  Tempest::Bind::UserTexture frame;
  };

struct UnitInfo::Stats : public PanelBase {
  Stats( Resource & res, GameObject& obj ):PanelBase(res), res(res), obj(obj){
    setLayout( Tempest::Vertical );

    Widget *w = new Widget();
    w->setLayout(Tempest::Horizontal);
    LineEdit *le = new LineEdit(res);
    std::string name = "$(unit/" +obj.getClass().name+")";
    le->setText( Lang::tr( name ) );
    le->setEditable( false );

    w->layout().add( new Widget() );
    w->layout().add(le);
    w->layout().add( new Widget() );
    layout().add(w);

    w = new Widget();
    layout().add(w);

    w->setLayout( Tempest::Horizontal );

    const char* icon[2] = {
      "gui/icon/atack",
      "gui/item/shield"
      };

    const wchar_t* hint[2] = {
      L"$(equipment/weapon0)",
      L"$(equipment/armor0)"
      };

    for( size_t i=0; i<2; ++i ){
      btns.push_back( std::shared_ptr<Btn>(new Btn(res)) );
      btns.back()->icon.data = res.pixmap(icon[i]);
      btns.back()->setHint( hint[i] );

      w->layout().add( btns.back().get() );
      }
    }

  void customEvent( Tempest::CustomEvent & ){
    }

  Resource  & res;
  GameObject& obj;

  std::vector< std::shared_ptr<Btn> > btns;
  };

struct UnitInfo::Production : public PanelBase {
  Production( Resource & res, GameObject& obj ):PanelBase(res), res(res), obj(obj){
    le = new ProgressBar(res);

    setLayout( Tempest::Vertical );

    top = new Widget();
    top->setLayout( Tempest::Horizontal );
    btns.push_back( std::shared_ptr<Btn>(new Btn(res)) );
    top->layout().add( btns[0].get() );
    top->layout().add( le );

    bottom = new Widget();
    bottom->setLayout( Tempest::Horizontal );
    for( size_t i=0; i<5; ++i ){
      btns.push_back( std::shared_ptr<Btn>(new Btn(res)) );
      bottom->layout().add( btns.back().get() );
      }

    bottom->layout().add( new Widget() );

    layout().add( top );
    layout().add( bottom );
    layout().add( new Widget() );
    }

  void customEvent( Tempest::CustomEvent & ){
    if( RecruterBehavior *b = obj.behavior.find<RecruterBehavior>() ){
      const std::vector<RecruterBehavior::Queue::Order>& q =
          b->orders()[0].orders;

      le->setVisible( q.size()>0 );

      if( q.size() ){
        int r = q[0].time;//obj.game().prototype( q[0] ).data.buildTime;
        le->setRange( 0, r );
        le->setValue( r - b->ctime() );
        }

      for( size_t i=0; i<q.size(); ++i ){
        if( i<btns.size() ){
          btns[i]->icon.data = res.pixmap(q[i].icon);
          btns[i]->setVisible(true);
          }
        }

      for( size_t i=q.size(); i<btns.size(); ++i ){
        btns[i]->setVisible(false);
        }

      }

    }

  ProgressBar * le;
  Resource  & res;
  GameObject& obj;

  Widget *top, *bottom;

  std::vector< std::shared_ptr<Btn> > btns;
  };


UnitInfo::UnitInfo(GameObject &obj, Resource & res )
         :obj(&obj), res(res) {
  setup( &obj );
  }

UnitInfo::UnitInfo(Resource &res):res(res) {
  obj = 0;
  setLayout( Tempest::Vertical );
  }

void UnitInfo::setup(GameObject *nobj) {
  obj = nobj;
  layout().removeAll();

  if( obj==0 )
    return;

  if( obj->behavior.find<RecruterBehavior>() )
    layout().add( new Production(res, *obj) );

  if( obj->behavior.find<WarriorBehavior>() )
    layout().add( new Stats(res, *obj) );
  }
