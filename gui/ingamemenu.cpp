#include "ingamemenu.h"

#include <MyWidget/Layout>
#include "panel.h"

#include "resource.h"
#include "gen/ui_ingamemenu.h"

#include <MyWidget/Painter>
#include "button.h"

InGameMenu::InGameMenu(Resource & res , Widget * ow):ModalWindow(res,ow) {
  frame.data = res.pixmap("gui/colors");

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
  menu.quit  ->clicked.bind( *findRoot(),    &Widget::deleteLater );
  }

void InGameMenu::mouseDownEvent(MyWidget::MouseEvent &e) {
  }

void InGameMenu::paintEvent( MyWidget::PaintEvent &e ) {
  {
  MyWidget::Painter p(e);
  p.setTexture( frame );
  p.setBlendMode( MyWidget::alphaBlend );
  p.drawRect( 0,0, w(), h(),
              0,2, 1,1 );
  }

  paintNested(e);
  }
