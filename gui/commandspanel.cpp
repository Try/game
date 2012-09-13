#include "commandspanel.h"

#include "util/math.h"
#include "gui/button.h"

#include <cmath>
#include "game/gameobject.h"
#include "resource.h"
#include "behavior/behaviormsgqueue.h"
#include "util/lexicalcast.h"

#include <array>

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

CommandsPanel::CommandsPanel( Resource & r, BehaviorMSGQueue &q )
              :Panel( r ), res(r), msg(q) {
  setLayout( new Layout() );
  layout().setMargin( 8 );
  u0 = 0;

  for( int i=0; i<25; ++i ){
    Button * b = new Button(res);
    b->setMaximumSize( MyWidget::SizePolicy::maxWidgetSize() );

    layout().add( b  );
    }
  }

void CommandsPanel::Layout::applyLayout() {
  size_t wc = 25;//widgets().size();
  if( wc==0 )
    return;

  size_t wcx = sqrt(wc),
         wcy = wcx;

  int w = owner()->w() - margin().xMargin(),
      h = owner()->h() - margin().yMargin();

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
        int x0 = r*w/wcx, x1 = (r+1)*w/wcx,
            y0 = i*h/wcy, y1 = (i+1)*h/wcy;

        placeIn( p[r][i],//widgets()[id],
                 x0+margin().left,
                 y0+margin().top,
                 x1-x0, y1-y0 );
        }
      }
  }


void CommandsPanel::bind( const std::vector<GameObject*> &u ) {
  for( size_t i=0; i<layout().widgets().size(); ++i )
    layout().widgets()[i]->deleteLater();

  if( u.size()==0 )
    return;

  u0 = u[0];
  setPage(0);
  }

void CommandsPanel::bind(const ProtoObject::Commans::Page &p) {
  onPageCanged();

  for( size_t i=0; i<layout().widgets().size(); ++i )
    layout().widgets()[i]->deleteLater();

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

    if( b ){
      b->x = p.btn[i].x;
      b->y = p.btn[i].y;
      b->setShortcut( MyWidget::Shortcut(b,p.btn[i].hotkey) );

      b->setMaximumSize( MyWidget::SizePolicy::maxWidgetSize() );
      layout().add( b  );
      }
    }
  }

void CommandsPanel::buyEvent(const std::string &unit) {
  msg.message( pl, Behavior::Buy,
               u0->x(), u0->y(),
               unit );
  }

void CommandsPanel::setPage(int page) {
  GameObject & obj = *u0;
  pl = obj.playerNum();

  if( obj.getClass().commands.pages.size()==0 )
    return;

  page = std::max(0, std::min<int>(page, obj.getClass().commands.pages.size() ));
  const ProtoObject::Commans::Page & p = obj.getClass().commands.pages[page];
  bind(p);
  }
