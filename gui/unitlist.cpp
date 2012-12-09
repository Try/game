#include "unitlist.h"

#include "gui/button.h"
#include <MyWidget/Layout>

#include "resource.h"
#include "game/gameobject.h"

struct UnitList::Btn : public Button {
  Btn( Resource & res ):Button(res){}

  GameObject * owner;
  };

struct UnitList::Lay : public MyWidget::Layout {
  void applyLayout(){
    }
  };

UnitList::UnitList( Resource & res ):res(res) {
  setLayout( new Lay() );
  }

void UnitList::setup(const std::vector<GameObject*> &u ) {
  layout().removeAll();
  btn.clear();
  const int sz = 40;

  for( size_t i=0; i<u.size(); ++i ){
    Btn *b = new Btn(res);
    b->icon.data = res.pixmap("gui/icon/"+u[i]->getClass().name);
    b->owner = u[i];
    b->setGeometry( sz*(i%10), sz*(i/10), sz, sz );

    layout().add( b );
    btn.push_back(b);
    }
  }

void UnitList::onUnitDied(GameObject &obj) {
  for( size_t i=0; i<btn.size(); ++i ){
    if( btn[i] && btn[i]->owner==&obj ){
      btn[i]->deleteLater();
      btn[i] = 0;
      }
    }
  }
