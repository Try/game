#include "mapselectmenu.h"

#include "resource.h"
#include <Tempest/LocalTexturesHolder>
#include <Tempest/Painter>
#include <Tempest/Layout>
#include "button.h"
#include <cmath>

struct Btn:Button{
  Btn(Resource &res):Button(res){}

  };

MapSelectMenu::MapSelectMenu(Resource &res, Widget *ow):ModalWindow(res, ow) {
  base     = res.ltexHolder.load("data/icons/maps/base.png");
  mPriview = res.ltexHolder.load("data/icons/maps/1.png");

  Button *bmenu = new Button(res);
  bmenu->clicked.bind(this, &Widget::deleteLater );

  Widget* mbox = new Widget();
  mbox->setLayout(Tempest::Horizontal);
  mbox->setSizePolicy( Tempest::Preferred, Tempest::FixedMin );
  mbox->setMinimumSize( bmenu->minSize() );
  mbox->layout().add(bmenu);
  mbox->layout().add( new Widget() );

  setLayout(Tempest::Vertical);
  layout().add( mbox );
  layout().add( new Widget() );

  isAnim  = false;
  mouseDx = 0;
  btns.push_back( Map{mPriview} );
  btns.push_back( Map{mPriview} );
  btns.push_back( Map{mPriview} );

  timer.timeout.bind(this, &MapSelectMenu::updateT);
  timer.start(10);
  }

void MapSelectMenu::paintEvent(Tempest::PaintEvent &e) {
  ModalWindow::paintEvent(e);

  Tempest::Painter p(e);

  p.setBlendMode( Tempest::alphaBlend );

  for( int i=0; i<3; ++i ){
    int dx   = i*base.width()*0.75 + mouseDx;
    float sz = 1.0 - 0.5*fabs(dx)/float(base.width());

    {
    Tempest::Texture2d &t = base;
    p.setTexture(t);
    p.drawRect( (w()-t.width()*sz)/2+dx, (h()-t.height()*sz)/2, t.width()*sz, t.height()*sz,
                0,0,t.width(),t.height());
    }
    {
    Tempest::Texture2d &t = mPriview;
    p.setTexture(t);
    p.drawRect( (w()-t.width()*sz)/2+dx, (h()-t.height()*sz)/2, t.width()*sz, t.height()*sz,
                0,0,t.width(),t.height());
    }
    }
  }

void MapSelectMenu::mouseDownEvent(Tempest::MouseEvent &e) {
  mpos     = e.pos();
  pressPos = e.pos();
  isAnim   = false;
  }

void MapSelectMenu::mouseDragEvent(Tempest::MouseEvent &e) {
  mouseDx += (e.pos().x-mpos.x);
  mpos = e.pos();
  update();
  }

void MapSelectMenu::mouseUpEvent( Tempest::MouseEvent &e ) {
  isAnim = true;
  update();

  if( (pressPos-e.pos()).manhattanLength()<15 )
    acceptMap();
  }

void MapSelectMenu::acceptMap() {
  acepted("td1_1.sav");
  deleteLater();
  }

void MapSelectMenu::updateT() {
  if( isAnim ){
    int dv = base.width()*0.75;
    int md = mouseDx%dv;
    if( md<0 )
      md += dv;

    if( md > dv/2 )
      mouseDx += (dv-md+19)/20; else
      mouseDx -= (md+19)/20;

    isAnim = (mouseDx%dv!=0);
    update();
    }
  }
