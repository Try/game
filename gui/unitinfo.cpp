#include "unitinfo.h"

#include "gen/ui_unitinfo.h"

#include "lineedit.h"
#include "button.h"
#include "progressbar.h"

#include "game/gameobject.h"
#include "behavior/recruterbehavior.h"

#include <MyWidget/Layout>
#include <sstream>

#include "game.h"

struct UnitInfo::Btn : public Button{
  Btn( Resource & res ):Button(res){
    const int sz = 40;
    setMinimumSize(sz, sz);
    setMaximumSize(sz, sz);
    setSizePolicy( MyWidget::FixedMax );
    }

  };

struct UnitInfo::Production : public MyWidget::Widget {
  Production( Resource & res, GameObject& obj ):res(res), obj(obj){
    le = new ProgressBar(res);

    setLayout( MyWidget::Vertical );

    top = new Widget();
    top->setLayout( MyWidget::Horizontal );
    btns.push_back( std::unique_ptr<Btn>(new Btn(res)) );
    top->layout().add( btns[0].get() );
    top->layout().add( le );

    bottom = new Widget();
    bottom->setLayout( MyWidget::Horizontal );
    for( size_t i=0; i<5; ++i ){
      btns.push_back( std::unique_ptr<Btn>(new Btn(res)) );
      bottom->layout().add( btns.back().get() );
      }

    bottom->layout().add( new Widget() );

    layout().add( top );
    layout().add( bottom );
    layout().add( new Widget() );
    }

  void customEvent( MyWidget::CustomEvent & ){
    if( RecruterBehavior *b = obj.behavior.find<RecruterBehavior>() ){
      const std::vector<std::string>& q = b->orders();

      le->setVisible( q.size()>0 );

      if( q.size() ){
        int r = obj.game().prototype( q[0] ).data.buildTime;
        le->setRange( 0, r );
        le->setValue( r - b->ctime() );
        }

      for( size_t i=0; i<q.size(); ++i ){
        if( i<btns.size() ){
          btns[i]->icon.data = res.pixmap("gui/icon/"+q[i]);
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

  std::vector< std::unique_ptr<Btn> > btns;
  };


UnitInfo::UnitInfo(GameObject &obj, Resource & res )
         :obj(&obj), res(res) {
  setup( &obj );
  }

UnitInfo::UnitInfo(Resource &res):res(res) {
  obj = 0;
  setLayout( MyWidget::Vertical );
  }

void UnitInfo::setup(GameObject *nobj) {
  obj = nobj;
  layout().removeAll();
  if( obj==0 )
    return;

  if( obj->behavior.find<RecruterBehavior>() )
    layout().add( new Production(res, *obj) );
/*
  UI::UnitInfo ui;
  ui.setupUi(this, res);

  ui.hpBox->setSizePolicy( ui.hp->sizePolicy() );
  ui.hpBox->setMaximumSize(500, 35);
  ui.hpBox->layout().setMargin(8);

  ui.hp->setEditable(0);
  ui.caption->setEditable(0);

  std::wstring s;
  s.assign( obj->getClass().name.begin(),
            obj->getClass().name.end() );
  ui.caption->setText( s );*/
  }
