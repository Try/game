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

  triangle = res.pixmap("gui/triangle");

  Button *bmenu = new Button(res);
  bmenu->clicked.bind(this, &Widget::deleteLater );
  bmenu->icon = triangle;
  bmenu->setMinimumSize(50,50);
  bmenu->setMaximumSize(50,50);

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
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/1.png"), L"td1_1.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/2.png"), L"td2.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/3.png"), L"td3.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/4.png"), L"td4.sav"} );

  timer.timeout.bind(this, &MapSelectMenu::updateT);
  timer.start(10);
  }

void MapSelectMenu::paintEvent(Tempest::PaintEvent &e) {
  ModalWindow::paintEvent(e);

  Tempest::Painter p(e);

  p.setBlendMode( Tempest::alphaBlend );

  for( size_t i=0; i<btns.size(); ++i ){
    {
      Tempest::Texture2d &t = base;
      p.setTexture(t);
      p.drawRect( rect(i),
                  t.size().toRect() );
    }
    {
      Tempest::Texture2d &t = btns[i].mPriview;
      p.setTexture(t);
      p.drawRect( rect(i,t),
                  t.size().toRect() );
    }
    }

  p.setTexture( triangle );
  p.drawRect(10, h()/2-triangle.h()/2, triangle.w(), triangle.h() );
  p.setFlip(1,0);
  p.drawRect( w()-triangle.w(), h()/2-triangle.h()/2, triangle.w(), triangle.h() );
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

  if( (pressPos-e.pos()).manhattanLength()<15 ){
    for( size_t i=0; i<btns.size(); ++i ){
      if( rect(i).contains(e.pos()) )
        acceptMap( btns[i].path );
      }
    }
  }

void MapSelectMenu::acceptMap( const std::wstring& str ) {
  acepted(str);
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

Tempest::Rect MapSelectMenu::rect( int i ) {
  return rect(i, base);
  }

Tempest::Rect MapSelectMenu::rect(int i, const Tempest::Texture2d &t) {
  int dx   = i*base.width()*0.75 + mouseDx;
  float sz = 1.0 - 0.5*fabs(dx)/float(base.width());

  return Tempest::Rect( (w()-t.width()*sz)/2+dx, (h()-t.height()*sz)/2,
                        t.width()*sz, t.height()*sz );
  }
