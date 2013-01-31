#include "commandspanel.h"

#include "util/math.h"
#include "gui/button.h"

#include <cmath>
#include "game/gameobject.h"
#include "resource.h"
#include "behavior/behaviormsgqueue.h"
#include "behavior/movebehavior.h"
#include "behavior/warriorbehavior.h"
#include "game.h"

#include "util/lexicalcast.h"

#include <array>
#include "util/array2d.h"

struct CommandsPanel::BtnBase: public Button{
  BtnBase( Resource& r ):Button(r), x(0), y(0) { }

  int x,y;
  };

class CommandsPanel::BuyBtn: public BtnBase{
  public:
    BuyBtn( Resource& r ):BtnBase(r){
      this->Button::clicked.bind( *this, &BuyBtn::proxyClick );
      }

    void proxyClick(){
      clicked( taget );
      }

    std::string taget;
    MyWidget::signal< const std::string& > clicked;
  };

class CommandsPanel::PageBtn: public BtnBase{
  public:
  PageBtn( Resource& r, int tg ):BtnBase(r), page(tg){
      this->Button::clicked.bind( *this, &PageBtn::proxyClick );
      }

    void proxyClick(){
      clicked( page );
      }

    std::string taget;
    int page;
    MyWidget::signal< int > clicked;
  };

class CommandsPanel::SpellBtn: public BtnBase{
  public:
    SpellBtn( Resource& r ):BtnBase(r){
      this->Button::clicked.bind( *this, &SpellBtn::proxyClick );
      texture.data = r.pixmap("gui/colors");

      coolDown = 0;
      }

    void proxyClick(){
      clicked( taget );
      }

    void paintEvent(MyWidget::PaintEvent &e){
      BtnBase::paintEvent(e);

      MyWidget::Painter p(e);
      p.setTexture( texture );
      p.setBlendMode( MyWidget::alphaBlend );

      p.drawRect( 0, h()-coolDown, w(), coolDown,
                  2,        4, 1, 1 );
      }

    void customEvent( MyWidget::CustomEvent & ){
      assert(u0);
      int maxT = u0->game().prototypes().spell(taget).coolDown;

      int mcoolDown = maxT;

      auto s = u0->player().selected();
      for( auto i=s.begin(); i!=s.end(); ++i ){
        GameObject & obj = **i;
        int t = obj.coolDown( tagetID );
        if( t>=0 )
          mcoolDown = std::min(t,mcoolDown);
        }

      mcoolDown = mcoolDown*h()/maxT;
      if( mcoolDown!=coolDown ){
        coolDown = mcoolDown;
        update();
        }
      }

    std::string taget;
    size_t      tagetID;
    GameObject  *  u0;
    MyWidget::Bind::UserTexture texture;
    int coolDown;

    MyWidget::signal< const std::string& > clicked;
  };

CommandsPanel::CommandsPanel( Resource & r, BehaviorMSGQueue &q )
              :Panel( r ), res(r), msg(q) {
  setLayout( new Layout() );
  layout().setMargin( 8 );
  u0 = 0;

  instaled = false;
  hook.mouseDown.bind( *this, &CommandsPanel::mouseDown    );
  hook.mouseUp  .bind( *this, &CommandsPanel::mouseUp      );
  hook.onRemove .bind( *this, &CommandsPanel::onRemoveHook );
  }

void CommandsPanel::Layout::applyLayout() {
  size_t wc = 25;//widgets().size();
  if( wc==0 )
    return;

  size_t wcx = 5,//sqrt(wc),
         wcy = 4;//wcx;

  //int w = owner()->w() - margin().xMargin(),
  //    h = owner()->h() - margin().yMargin();

  array2d<BtnBase*> p;
  p.resize(wcx, wcy);

  for( size_t i=0; i<widgets().size(); ++i ){
    BtnBase * b = dynamic_cast<BtnBase*>( widgets()[i] );
    if( b ){
      int x = b->x, y = b->y;

      if( p[x][y]!=0 ){
        int d = p.width()*p.height();

        for( int i=0; i<p.width(); ++i )
          for( int r=0; r<p.height(); ++r ){
            if( p[i][r]==0 && abs(i-x)+abs(r-y)<d ){
              x = i;
              y = r;
              d = abs(i-x)+abs(r-y);
              }
            }
        }

      if( p[x][y]==0 ){
        p[x][y] = b;
        }
      }
    }

  for( size_t i=0; i<wcy; ++i )
    for( size_t r=0; r<wcx; ++r ){
      //size_t id = i*wcx+r;
      if( p[r][i]/*id < widgets().size()*/ ){
        //int x0 = r*w/wcx, x1 = (r+1)*w/wcx,
        //    y0 = i*h/wcy, y1 = (i+1)*h/wcy;
        int sz = 50;
        int x0 = r*sz+2,
            y0 = i*sz+2;

        placeIn( p[r][i],//widgets()[id],
                 x0+margin().left,
                 y0+margin().top,
                 sz, sz );
        }
      }
  }


void CommandsPanel::bind(GameObject *u ) {
  if( u0==u )
    return;
  u0 = u;

  if( instaled )
    msg.game.removeHook( &hook );

  for( size_t i=0; i<layout().widgets().size(); ++i )
    layout().widgets()[i]->deleteLater();

  if( u==0 )
    return;

  setPage(0);
  }

void CommandsPanel::bindPage( const ProtoObject::Commans::Page &p ) {
  for( size_t i=0; i<p.btn.size(); ++i ){
    BtnBase * b = 0;//new Button(res);

    if( p.btn[i].action == ProtoObject::CmdButton::Buy ){
      BuyBtn * btn   = new BuyBtn(res);
      btn->taget     = p.btn[i].taget;
      btn->icon.data = res.pixmap( "gui/icon/"+p.btn[i].taget );
      btn->clicked.bind(*this, &CommandsPanel::buyEvent );
      b = btn;
      }

    if( p.btn[i].action == ProtoObject::CmdButton::Page ){
      PageBtn * btn   = new PageBtn(res, Lexical::cast<int>(p.btn[i].taget) );
      btn->taget     = p.btn[i].taget;
      if( p.btn[i].icon.size() )
        btn->icon.data = res.pixmap( p.btn[i].icon ); else
        btn->icon.data = res.pixmap( "gui/icon/castle" );
      btn->clicked.bind(*this, &CommandsPanel::setPage );
      b = btn;
      }

    if( p.btn[i].action == ProtoObject::CmdButton::Build ){
      BuyBtn * btn   = new BuyBtn(res);
      btn->taget     = p.btn[i].taget;
      btn->icon.data = res.pixmap( "gui/icon/"+p.btn[i].taget );
      btn->clicked.bind(*this, &CommandsPanel::buyEvent );
      b = btn;
      }

    if( p.btn[i].action == ProtoObject::CmdButton::CastToGround ){
      SpellBtn * btn   = new SpellBtn(res);
      btn->taget       = p.btn[i].taget;
      btn->tagetID     = msg.game.prototypes().spell( btn->taget ).id;
      btn->u0          = u0;

      if( p.btn[i].icon.size() )
        btn->icon.data = res.pixmap( p.btn[i].icon ); else
        btn->icon.data = res.pixmap( "gui/icon/castle" );

      btn->clicked.bind(*this, &CommandsPanel::setupHook );
      b = btn;
      }

    if( b ){
      b->setHint( p.btn[i].hint );
      b->x = p.btn[i].x;
      b->y = p.btn[i].y;
      b->setShortcut( MyWidget::Shortcut(b,p.btn[i].hotkey) );

      b->setMaximumSize( MyWidget::SizePolicy::maxWidgetSize() );
      layout().add( b  );
      }
    }
  }

void CommandsPanel::bind( const ProtoObject::Commans::Page &p ) {
  onPageCanged();

  for( size_t i=0; i<layout().widgets().size(); ++i )
    layout().widgets()[i]->deleteLater();

  bindPage(p);
  }

void CommandsPanel::buyEvent(const std::string &unit) {
  msg.message( pl, Behavior::Buy,
               u0->x(), u0->y(),
               unit,
               pl );
  }

void CommandsPanel::moveClick() {
  if( MoveBehavior * m = u0->behavior.find<MoveBehavior>() ){
    m->setupMoveHook();
    }
  }

void CommandsPanel::stopClick() {
  u0->game().message( u0->playerNum(),
                      AbstractBehavior::Cancel,
                      0, 0 );
  }

void CommandsPanel::atkClick() {
  if( WarriorBehavior * m = u0->behavior.find<WarriorBehavior>() ){
    m->aClick();
    }
  }

void CommandsPanel::setPage(int page) {
  GameObject & obj = *u0;
  pl = obj.playerNum();

  if( obj.getClass().commands.pages.size()==0 ){
    bindStartPage(0);
    return;
    }

  page = std::max(0, std::min<int>(page, obj.getClass().commands.pages.size() ));
  const ProtoObject::Commans::Page & p = obj.getClass().commands.pages[page];

  if( page==0 ){
    bindStartPage(&p);
    } else {
    bind(p);
    }
  }

void CommandsPanel::bindStartPage(const ProtoObject::Commans::Page *p) {
  onPageCanged();

  for( size_t i=0; i<layout().widgets().size(); ++i )
    layout().widgets()[i]->deleteLater();

  GameObject & obj = *u0;

  const char k[] = {'m', 's', 'h', 'p', 'a'};
  const char* icon[] = {
    "gui/icon/move",
    "gui/icon/stop",
    "gui/icon/hold",
    "gui/icon/patrul",
    "gui/icon/atack"
    };

  const wchar_t* hint[] = {
    L"$(move)",
    L"$(stop)",
    L"$(hold)",
    L"$(patrul)",
    L"$(atack)"
    };

  BtnBase* btn[5] = {};

  for( int i=0; i<5; ++i ){
    BtnBase * b = new BuyBtn(res);
    btn[i] = b;

    b->x = i;
    b->y = 0;
    b->icon.data = res.pixmap( icon[i] );
    b->setHint( hint[i] );

    MyWidget::KeyEvent::KeyType t = MyWidget::Event::K_A;
    t = MyWidget::KeyEvent::KeyType( int(t) + k[i] - 'a' );

    b->setShortcut( MyWidget::Shortcut(b, t) );

    b->setMaximumSize( MyWidget::SizePolicy::maxWidgetSize() );
    layout().add( b  );
    }

  btn[0]->clicked.bind(*this, &CommandsPanel::moveClick );
  btn[1]->clicked.bind(*this, &CommandsPanel::stopClick );

  btn[4]->clicked.bind(*this, &CommandsPanel::atkClick  );

  if( obj.behavior.find<WarriorBehavior>()==0 ){
    btn[4]->deleteLater();
    if( obj.behavior.find<MoveBehavior>()==0 )
      btn[1]->deleteLater();
    }

  if( obj.behavior.find<MoveBehavior>()==0 ){
    btn[0]->deleteLater();
    btn[2]->deleteLater();
    btn[3]->deleteLater();
    }

  if( p )
    bindPage(*p);
  }

void CommandsPanel::setupHook(const std::string &s) {
  if( !instaled && u0 ){
    instaled    = u0->game().instalHook( &hook );
    spellToCast = s;
    }
  }

void CommandsPanel::mouseDown(MyWidget::MouseEvent &e) {
  e.accept();
  }

void CommandsPanel::mouseUp(MyWidget::MouseEvent &e) {
  if( e.button==MyWidget::MouseEvent::ButtonLeft && u0 ){
    u0->game().message( u0->playerNum(),
                        BehaviorMSGQueue::SpellCast,
                        u0->world().mouseX(),
                        u0->world().mouseY(),
                        spellToCast
                        );
    }

  u0->game().removeHook( &hook );
  }

void CommandsPanel::onRemoveHook() {
  instaled = false;
  }
