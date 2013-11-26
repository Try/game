#include "mapselectmenu.h"

#include "resource.h"
#include <Tempest/LocalTexturesHolder>
#include <Tempest/Painter>
#include <Tempest/Layout>

#include "button.h"
#include "listbox.h"

#include "lang/lang.h"
#include "game/player.h"

#include "gamesettings.h"

#include <cmath>

struct Btn:Button{
  Btn(Resource &res):Button(res){}
  };

struct MapSelectMenu::ColorBtn: Button{
  ColorBtn( Resource & res ):Button(res){
    setMinimumSize(75,75);
    setMaximumSize(75,75);

    sp   = res.pixmap("gui/plColor");
    cl   = Tempest::Color(1);
    }

  void paintEvent(Tempest::PaintEvent &e){
    Tempest::Painter p(e);

    drawBack(p);

    p.setColor(cl);
    p.setTexture(sp);

    if( GameSettings::color == cl ){
      p.drawRect( size().toRect(),
                  sp.size().toRect() );
      } else {
      p.drawRect( Tempest::Rect(3,3, w()-6, h()-6),
                  sp.size().toRect() );
      }

    p.setColor( Tempest::Color(1) );

    p.setBlendMode( Tempest::alphaBlend );
    drawFrame(p);
    finishPaint();
    }

  void emitClick(){
    Button::emitClick();
    onClicked( cl );
    update();
    }

  Tempest::signal<Tempest::Color> onClicked;

  Tempest::Sprite sp;
  Tempest::Color  cl;
  };

struct MapSelectMenu::ColorChoser: AbstractListBox {
  ColorChoser(Resource & r):AbstractListBox(r), res(r){
    setMinimumSize(100,100);
    setMaximumSize(100,100);

    sp   = res.pixmap("gui/plColor");
    }

  void paintEvent(Tempest::PaintEvent &e){
    Tempest::Painter p(e);

    drawBack(p);

    p.setTexture(sp);
    p.setColor( GameSettings::color );
    p.drawRect( Tempest::Rect(3,3, w()-6, h()-6),
                sp.size().toRect() );
    p.setColor( Tempest::Color(1) );

    p.setBlendMode( Tempest::alphaBlend );
    drawFrame(p);
    }

  Tempest::Widget *createDropList(){
    Panel *p = new Panel(res);
    p->resize(250,250);
    p->setLayout( Tempest::Vertical );
    p->setMargin(10);

    for( int i=0; i<3; ++i ){
      Widget* w = new Widget();
      w->setLayout(Tempest::Horizontal);
      for( int r=0; r<3; ++r ){
        ColorBtn *b = new ColorBtn(res);
        b->cl = Player::colors[(i*3+r)%9];
        b->onClicked.bind( onClicked );

        w->layout().add( b );
        }

      p->layout().add( w );
      }

    p->setPosition( mapToRoot( Tempest::Point(-p->w(), h() )) );
    return p;
    }

  Tempest::signal<Tempest::Color> onClicked;
  Resource & res;
  Tempest::Sprite sp;
  };

struct MapSelectMenu::Options: AbstractListBox {
  Options(Resource & r):AbstractListBox(r), res(r){}

  Tempest::Widget *createDropList(){
    Panel *p = new Panel(res);
    p->resize(140,150);
    p->setLayout( Tempest::Vertical );
    p->setMargin(10);

    ColorChoser *ch = new ColorChoser(res);
    ch->onClicked.bind( onColor );
    p->layout().add( ch );

    ListBox *l = new ListBox(res);
    std::vector< std::wstring > d;
    d.push_back( Lang::tr(L"$(difficulty/easy)")   );
    d.push_back( Lang::tr(L"$(difficulty/medium)") );
    d.push_back( Lang::tr(L"$(difficulty/hard)")   );

    l->setItemList(d);
    l->setCurrentItem( GameSettings::difficulty );
    l->onItemSelected.bind( onItemSelected );

    p->layout().add( l );

    p->setPosition( mapToRoot( Tempest::Point(-p->w(), h() )) );

    return p;
    }

  Tempest::signal<int> onItemSelected;
  Tempest::signal<Tempest::Color> onColor;

  Resource & res;
  };

MapSelectMenu::MapSelectMenu(Resource &res, Widget *ow):ModalWindow(res, ow), res(res) {
  base     = res.ltexHolder.load("data/icons/maps/base.png");
  mPriview = res.ltexHolder.load("data/icons/maps/1.png");

  triangle = res.pixmap("gui/triangle");
  GameSettings::load();

  Button *bmenu = new Button(res);
  bmenu->clicked.bind(this, &Widget::deleteLater );
  bmenu->icon = triangle;
  bmenu->setMinimumSize(75,75);
  bmenu->setMaximumSize(75,75);
  bmenu->frame   = Tempest::Sprite();
  bmenu->back[0] = Tempest::Sprite();
  bmenu->back[1] = Tempest::Sprite();

  Widget* mbox = new Widget();
  mbox->setLayout(Tempest::Horizontal);
  mbox->setSizePolicy( Tempest::Preferred, Tempest::FixedMin );
  mbox->setMinimumSize( bmenu->minSize() );
  mbox->layout().add(bmenu);
  mbox->layout().add( new Widget() );

  Options *optMenu = new Options(res);
  optMenu->clicked.bind(this, &MapSelectMenu::showOptions );
  optMenu->onItemSelected.bind(this, &MapSelectMenu::setDificulty);
  optMenu->onColor.bind(this, &MapSelectMenu::setColor );

  optMenu->icon = res.pixmap("gui/icon/settings");
  optMenu->frame   = Tempest::Sprite();
  optMenu->back[0] = Tempest::Sprite();
  optMenu->back[1] = Tempest::Sprite();

  optMenu->setMinimumSize(75,75);
  optMenu->setMaximumSize(75,75);
  mbox->layout().add(optMenu);

  setLayout(Tempest::Vertical);
  layout().add( mbox );

  isAnim  = false;
  mouseDx = 0;
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/1.png"), L"td1_1.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/2.png"), L"td2.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/3.png"), L"td3.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/4.png"), L"td4.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/5.png"), L"td5.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/6.png"), L"td6.sav"} );
  btns.push_back( Map{ res.ltexHolder.load("data/icons/maps/7.png"), L"td7.sav"} );

  timer.timeout.bind(this, &MapSelectMenu::updateT);
  timer.start(10);
  }

MapSelectMenu::~MapSelectMenu() {
  isAnim  = false;
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
  if( rect(0).x+rect(0).w<w()/2 )
    p.drawRect( leftBtn() );

  p.setFlip(1,0);

  if( rect(btns.size()-1).x>w()/2 )
    p.drawRect( rightBtn() );
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

  if( leftBtn().contains(e.pos()) ){
    return;
    }

  if( rightBtn().contains(e.pos()) ){
    return;
    }

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

void MapSelectMenu::showOptions() {
  //new Options(res);
  }

void MapSelectMenu::setColor(const Tempest::Color &cl) {
  GameSettings::color = cl;
  GameSettings::save();
  }

void MapSelectMenu::setDificulty(int d) {
  GameSettings::difficulty = d;
  GameSettings::save();
  }

Tempest::Rect MapSelectMenu::leftBtn() {
  return Tempest::Rect( 10, h()/2-triangle.h()/2, triangle.w(), triangle.h() );
  }

Tempest::Rect MapSelectMenu::rightBtn() {
  return Tempest::Rect( w()-triangle.w(), h()/2-triangle.h()/2, triangle.w(), triangle.h() );
  }
