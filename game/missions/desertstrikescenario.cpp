#include "desertstrikescenario.h"

#include "desertstrikescenariowidgets.h"

#include "game.h"
#include "lang/lang.h"

#include "gui/gen/ui_toppanel.h"
#include "gui/missiontargets.h"
#include "gui/unitview.h"

#include "util/bytearrayserialize.h"

#include "util/math.h"
#include "algo/algo.h"

DesertStrikeScenario::DesertStrikeScenario(Game &g, MainGui &ui, BehaviorMSGQueue &msg)
  :Scenario(g,ui, msg){
  tNum      = 0;
  interval  = 1000;
  isTestRun = 0;

  isMouseTracking = false;

  plC[0].units["pikeman"] = 3;
  plC[1].units["pikeman"] = 3;

  if( !isTestRun ){
    game.player(1).setGold(350);
    game.player(2).setGold(350);
    }

  gui.update();
  g.prototypes().load("campagin/td.xml");
  }

DesertStrikeScenario::~DesertStrikeScenario() {
  game.prototypes().unload();
  }

void DesertStrikeScenario::mouseDownEvent( Tempest::MouseEvent &e ) {
  mpos3d          = unProjectRz( e.x, e.y, moveZ );
  isMouseTracking = true;
  }

void DesertStrikeScenario::mouseUpEvent( Tempest::MouseEvent & e ) {
  F3 v = unProject( e.x, e.y );
  isMouseTracking = false;

  if( spellToCast.size() ){
    World  &w  = game.curWorld();
    w.emitHudAnim( "hud/blink",
                   (v.data[0]),
                   (v.data[1]),
                   0.01 );
    spellToCast.clear();
    return;
    }

  World  &world  = game.curWorld();
  Player &player = game.player();

  updateMousePos(e);

  if( e.button==Tempest::MouseEvent::ButtonLeft && player.editObj ){
    msg.message( player.number(), Behavior::EditNext );
    }

  if( e.button==Tempest::MouseEvent::ButtonRight && player.editObj ){
    msg.message( player.number(), Behavior::EditDel );
    }

  world.clickEvent( World::coordCastD(v.data[0]),
                    World::coordCastD(v.data[1]),
                    e );
  }

void DesertStrikeScenario::mouseMoveEvent( Tempest::MouseEvent &e ) {
  if( spellToCast.size() )
    return;

  if( !isMouseTracking && gui.mouseMoveEvent(e) ){
    acceptMouseObj = false;
    return;
    }

  if( isMouseTracking ){
    F3 m = unProject( e.x, e.y, moveZ );
    game.curWorld().moveCamera( mpos3d.data[0]-m.data[0],
                                mpos3d.data[1]-m.data[1]);
    mpos3d = unProject( e.x, e.y, moveZ );
    }

  updateMousePos(e);
  }

void DesertStrikeScenario::customEvent(const std::vector<char> &m) {
  ByteArraySerialize s( m );

  int pl;
  char ch;
  std::string name;

  s.read(pl);
  s.read(name);
  s.read(ch);

  if( ch=='b' ){
    int g = game.prototype(name).data.gold;
    if( game.player(pl+1).gold() >= g ){
      ++plC[ pl ].units[name];
      game.player(pl+1).setGold( game.player(pl+1).gold() -
                                 game.prototype(name).data.gold );

      //debug
      if( isTestRun )
        ++plC[ 1 ].units[name];
      }
    } else
  if( ch=='s' && plC[ pl ].units[name]>0 ){
    --plC[ pl ].units[name];
    game.player(pl+1).setGold( game.player(pl+1).gold() +
                               (75*game.prototype(name).data.gold)/100 );

    //debug    
    if( isTestRun )
      --plC[ 1 ].units[name];
    } else
  if( ch=='g' && plC[pl].queue.size()==0 ){
    plC[pl].queue.push_back(name);
    grade( plC[pl], name );
    }else
  if( ch=='m' ){
    spellToCast = "spellToCast";
    //game.curWorld().emitHudAnim();
    }

  mmap->updateValues();
  }

void DesertStrikeScenario::grade( PlInfo &pl, const std::string &/*grade*/ ){
  pl.maxBTime = 500;
  pl.btime    = pl.maxBTime;
  }

void DesertStrikeScenario::onUnitDied(GameObject &obj) {
  if( obj.playerNum()>=1 && obj.playerNum()<=2 )
    --plC[obj.playerNum()-1].realCount[ obj.getClass().name ];
  }

void DesertStrikeScenario::tick() {
  int w = game.curWorld().terrain().width() *Terrain::quadSize;
  int h = game.curWorld().terrain().height()*Terrain::quadSize;

  int p1 = 10,
      p2 = 80-p1;
  if( tNum%interval==0 ){
    mkUnits( 1,
             p1*w/80, p1*h/80,
             p2*w/80, p2*h/80,
             0 );
    }

  if( tNum%interval==0 ){
    mkUnits( 2,
             p2*w/80, p2*h/80,
             p1*w/80, p1*h/80,
             1 );
    }

  if( tNum%interval==0 ){
    if( !isTestRun )
      aiTick(1);
    }

  ++tNum;
  if( tNum/interval>=1 ){
    tNum -= interval;
    //aiTick(1);
    }

  for( int i=0; i<2; ++i ){
    if( plC[i].btime>0 ){
      mmap->update();
      --plC[i].btime;
      } else
    if( plC[i].btime==0 ){
      if( plC[i].maxBTime ){
        plC[i].maxBTime = 0;
        mmap->update();
        }

      if( plC[i].queue.size() ){
        grade( plC[i], plC[i].queue.front() );

        if( plC[i].queue.front()=="house" ){
          ++plC[i].economyGrade;
          }

        for( size_t r=0; r+1<plC[i].queue.size(); ++r ){
          plC[i].queue[r] = plC[i].queue[r+1];
          }
        plC[i].queue.pop_back();
        }
      }

    if( tNum%10==0 ){
      if( !(plC[i].btime && plC[i].queue.size() && plC[i].queue[0]=="house") )
        game.player(i+1).addGold( 1+plC[i].economyGrade );
      }
    }
  }

void DesertStrikeScenario::mkUnits( int pl,
                                    int   x, int   y,
                                    int tgX, int tgY,
                                    bool rev ){
  --pl;

  std::map<std::string, int>::iterator u, e = plC[pl].units.end();
  int count = 0;

  for( u = plC[pl].units.begin(); u!=e; ++u )
    count += u->second;

  int qc = 1;
  //std::max(Math::sqrt(count)-1,1);
  while( qc*qc<count )
    ++qc;

  int id = 0;
  for( u = plC[pl].units.begin(); u!=e; ++u ){
    int& c = plC[pl].realCount[u->first];
    for( int i=0; i<u->second; ++i ){
      if( 1 || c < 3*u->second ){
        GameObject& obj = game.curWorld().addObject(u->first, pl+1);
        ++c;
        ++id;

        if( rev ){
          obj.setPosition( x+(qc/2 - id%qc)*Terrain::quadSize,
                           y+(qc/2 - id/qc)*Terrain::quadSize );
          } else {
          obj.setPosition( x+(id%qc - qc/2)*Terrain::quadSize,
                           y+(id/qc - qc/2)*Terrain::quadSize );
          }

        obj.behavior.message( Behavior::MoveSingle, tgX, tgY );
        obj.behavior.message( Behavior::AtackMove,  tgX, tgY );
        }
      }
    }
  }

void DesertStrikeScenario::onStartGame() {
  int w = game.curWorld().terrain().width() *Terrain::quadSize;
  int h = game.curWorld().terrain().height()*Terrain::quadSize;

  /*
  {
    GameObject& obj = game.curWorld().addObject("tower", 1);
    int p = 40-9;
    obj.setPosition( p*w/80, p*h/80 );
    }

  {
    GameObject& obj = game.curWorld().addObject("tower", 2);

    int p = 40+8;
    obj.setPosition( p*w/80, p*h/80 );
    }*/

  World &wx = game.curWorld();
  for( size_t i=0; i<wx.activeObjects().size(); ++i ){
    GameObject & obj = *wx.activeObjects()[i];
    int w = game.curWorld().terrain().width(),
        h = game.curWorld().terrain().height(),
        x = w*Terrain::quadSize-obj.x(),
        y = obj.y();

    if( x*h > y*w ){
      obj.setPlayer(1);
      } else {
      obj.setPlayer(2);
      }
    }

  {
    GameObject& obj = game.curWorld().addObject("castle", 1);
    int p = 10;
    obj.setPosition( p*w/80, p*h/80 );
    }

  {
    GameObject& obj = game.curWorld().addObject("castle", 2);
    int p = 80-10;
    obj.setPosition( p*w/80, p*h/80 );
    }
  }

void DesertStrikeScenario::setupUI( InGameControls *mw, Resource &res ) {
  using namespace Tempest;

  mainWidget = mw;
  //currPl     = 1;

  showEditPanel = Shortcut(mainWidget, Tempest::KeyEvent::K_F9);
  showSettings  = Shortcut(mainWidget, Tempest::KeyEvent::K_F8);

  mainWidget->setLayout( Tempest::Vertical );
  mainWidget->layout().setMargin( Tempest::Margin(4) );
  mainWidget->useScissor( false );

  Widget * top = new Widget(),
         * cen = new Widget();

  { SizePolicy p;
    p.typeV = Tempest::Expanding;
    cen->setSizePolicy(p);
    }

  mainWidget->layout().add( top );
  mainWidget->layout().add( cen );

  mainWidget->layout().add( createConsole(mainWidget, msg) );

  top->setLayout( Tempest::Horizontal );

  SizePolicy p;
  p.maxSize.h = 30;
  p.typeV = Tempest::FixedMax;

  {
    UI::TopPanel p;
    p.setupUi( top, res );
    //p.fullScr->clicked.bind( toogleFullScreen );
    p.menu->clicked.bind( *this, &DesertStrikeScenario::showMenu );
    p.menu->setShortcut( Tempest::Shortcut(p.menu, Tempest::KeyEvent::K_ESCAPE) );

    p.frmEdit->clicked.bind( *this, &DesertStrikeScenario::showFormBuilder );
    gold = p.gold;
    lim  = p.lim;

    gold->icon.data = res.pixmap("gui/icon/gold");
    lim-> icon.data = res.pixmap("gui/icon/house");

    gold->setHint(L"$(gold)");
    lim ->setHint(L"$(units_limit)");
    lim->setVisible(0);

    p.frmEdit->setVisible(0);
    p.fullScr->setVisible(0);
    }

  p.maxSize.h = gold->sizePolicy().maxSize.h;
  top->setSizePolicy(p);

  editPanel     = createEditPanel(mainWidget, res);
  settingsPanel = createSettingsPanel(mainWidget, res);

  cen->setLayout( Tempest::Horizontal );
  cen->layout().add( new MissionTargets(game, res) );
  cen->layout().add( new SpellPanel(res, game, plC[0]) );

  Tempest::Widget * box = new Tempest::Widget();
  box->layout().add( editPanel );
  box->layout().add( settingsPanel );
  cen->layout().add( box );
  cen->layout().add( new UpgradePanel(res, game, plC[0], buyUnitPanel) );

  cen->useScissor( false );
  box->useScissor( false );

  editPanel->setVisible(0);
  //settingsPanel->setVisible(0);

  showEditPanel.activated.bind( *this, &DesertStrikeScenario::toogleEditPanel );
  showSettings. activated.bind( *this, &DesertStrikeScenario::toogleSettingsPanel );
  }

Tempest::Widget *DesertStrikeScenario::createConsole( InGameControls *mainWidget,
                                                      BehaviorMSGQueue &  ) {
  using namespace Tempest;

  Tempest::Widget* console = new Widget();
  console->setMaximumSize( SizePolicy::maxWidgetSize().w, 220 );
  console->setMinimumSize( 0, 200);

  SizePolicy p;
  p.minSize = Tempest::Size(200, 200);
  p.maxSize = Tempest::Size(200, 200);

  console->setLayout( Tempest::Horizontal );
  console->layout().setSpacing(6);

  Panel * img = new Panel( res );
  img->setSizePolicy(p);

  mmap = new Minimap(res,game, plC[0]);
  mmap->base->renderScene.bind( mainWidget->renderScene );
  mainWidget->updateView.bind( *mmap->base, &UnitView::updateView );
  mmap->base->onClick.bind(*mmap, &Minimap::hideInfo);

  minimap = mmap;
  minimap->setTexture( res.texture("grass/diff") );
  minimap->mouseEvent.bind( mainWidget->minimapEvent );

  img->setLayout( Vertical );
  img->layout().add( minimap );
  img->layout().setMargin(8);

  console->layout().add( img );
  Widget *w = new Widget();
  w->setSizePolicy( Tempest::Expanding );
  console->layout().add( w );

  buyUnitPanel = new BuyUnitPanel( res, game, plC[0], mmap );
  console->layout().add( buyUnitPanel );
  console->useScissor(false);

  return console;
  }

int DesertStrikeScenario::PlInfo::getParam(const std::string &p) const {
  if( p=="atack" )
    return atkGrade;

  if( p=="armor" )
    return armorGrade;

  if( p=="house" )
    return economyGrade;

  std::map<std::string, int>::const_iterator i = units.find(p);
  if( i==units.end() )
    return 0;

  return units.at(p);
  }

void DesertStrikeScenario::aiTick( int pl ) {
  int c = 0;
  for( auto i=plC[pl].units.begin(); i!=plC[pl].units.end(); ++i )
    c += (i->second);

  if( std::min(4, c/5) > plC[pl].economyGrade ){
    if( plC[pl].queue.size()==0 ){
      plC[pl].queue.push_back("house");
      grade( plC[pl], "house" );
      }
    }

  struct BuildElement{
    std::string src;
    int scount;
    std::string dst;
    int dcount;
    };

  BuildElement b[] = {
    {"pikeman",      3, "incvisitor",   1 },
    {"pikeman",      5, "gelion",       1 },
    {"gelion",       2, "fire_mage",    1 },
    {"incvisitor",   1, "pikeman",      3 },
    {"incvisitor",   6, "fire_element", 1 },

    {"incvisitor",   3, "fire_mage",  3 },
    {"fire_mage",    5, "golem",      2 },
    {"balista",      1, "incvisitor", 1 },

    {"fire_element", 1, "golem",      1 },
    {"golem",        1, "balista",    2 },
    {"golem",        4, "golem",      3 },
    {"golem",        1, "incvisitor", 1 },
    {"", 0, "", 0}
  };

  int count = 0;
  for( ; b[count].src.size(); ++count )
    ;

  for( int pass=1; pass<4; ++pass ){
    for( int i=0; i<2; ++i )
      if( pl!=i ){
        for( int r=count-1; r>=0; --r ){
          const BuildElement& e = b[r];

          int c = plC[i].units[e.src];
          c*=e.dcount;
          c*=pass;
          c/=e.scount;

          for( int q=plC[ pl ].units[e.dst]; q<c; ++q ){
            int gold = game.prototype(e.dst).data.gold;
            if( game.player(pl+1).gold() >= gold ){
              ++plC[ pl ].units[e.dst];
              game.player(pl+1).setGold( game.player(pl+1).gold() - gold );
              }
            }
          }
        }
    }
  }
