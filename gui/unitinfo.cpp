#include "unitinfo.h"

#include "gen/ui_unitinfo.h"

#include "lineedit.h"
#include "game/gameobject.h"

#include <MyWidget/Layout>

UnitInfo::UnitInfo(GameObject &obj, Resource & res ) {
  UI::UnitInfo ui;
  ui.setupUi(this, res);

  ui.hpBox->setSizePolicy( ui.hp->sizePolicy() );
  ui.hpBox->setMaximumSize(500, 35);
  ui.hpBox->layout().setMargin(8);

  ui.hp->setEditable(0);
  ui.caption->setEditable(0);

  std::wstring s;
  s.assign( obj.getClass().name.begin(),
            obj.getClass().name.end() );
  ui.caption->setText( s );
  }
