#include "ingamemenu.h"

#include <MyWidget/Layout>
#include "panel.h"

#include "resource.h"
#include "gen/ui_ingamemenu.h"

#include <MyWidget/Painter>
#include "button.h"

InGameMenu::InGameMenu( Resource &res , Widget * ow):ModalWindow(res,ow) {
  layout().add( new Widget() );
  Panel *p = new Panel(res);
  UI::InGameMenu menu;
  menu.setupUi(p, res);
  p->layout().setMargin(8);

  setLayout( MyWidget::Vertical );
  p->setMaximumSize(175, 175);
  p->setSizePolicy( MyWidget::FixedMax );

  layout().add(p);
  layout().add( new Widget() );
  layout().add( new Widget() );

  menu.resume->clicked.bind( (Widget&)*this, &Widget::deleteLater );
  menu.quit  ->clicked.bind( quit );
  menu.save  ->clicked.bind( save );
  menu.load  ->clicked.bind( load );
  }
