#include "desertstrikescenario.h"

#include "desertstrikescenariowidgets.h"

#include "game.h"
#include "lang/lang.h"

#include "gui/gen/ui_toppanel.h"
#include "gui/missiontargets.h"
#include "gui/unitview.h"
#include "gui/gamemessages.h"

#include "util/bytearrayserialize.h"

#include "util/math.h"
#include "algo/algo.h"

const char * DesertStrikeScenario::units[3][4] = {
  {"gelion", "pikeman", "incvisitor", "water_mage"},
  {"fire_mage", "balista"},
  {"fire_element", "golem"}
  };

DesertStrikeScenario::DesertStrikeScenario(Game &g, MainGui &ui, BehaviorMSGQueue &msg)
  :Scenario(g,ui, msg){
  tNum      = 0;
  interval  = 25*Game::ticksPerSecond;
  isTestRun = 0;
  plCenter  = 0;

  unitToView   = 0;
  hasVTracking = 1;

  isMouseTracking = false;

  player(1).units["pikeman"] = 3;
  player(2).units["pikeman"] = 3;

  if( !isTestRun ){
    player(1).setGold(250);
    player(2).setGold(250);
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

  unitToView   = 0;
  hasVTracking = 0;
  }

void DesertStrikeScenario::mouseUpEvent( Tempest::MouseEvent & e ) {
  F3 v = unProject( e.x, e.y );
  isMouseTracking = false;

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
  if( !isMouseTracking && gui.mouseMoveEvent(e) ){
    acceptMouseObj = false;
    return;
    }

  acceptMouseObj = true;

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
    if( tierOf(name.c_str()) <= player().castleGrade ){
      int g = game.prototype(name).data.gold;
      if( player(pl+1).gold() >= g ){
        ++player(pl+1).units[name];
        player(pl+1).setGold( player(pl+1).gold() -
                              game.prototype(name).data.gold );

        //debug
        if( isTestRun )
          ++player(2).units[name];

        GameMessages::message( L"$(message/new_unit)",
                               res.pixmap("gui/icon/"+name) );
        } else {
        GameMessages::message( L"$(message/no_gold)",
                               res.pixmap("gui/icon/gold") );
        }
      } else {
      GameMessages::message( L"$(desertstrike/need_castle_grade)",
                             res.pixmap("gui/icon/castle") );
      }
    } else
  if( ch=='s' && player( pl+1 ).units[name]>0 ){
    GameMessages::message( L"$(desertstrike/sell_unit)",
                           res.pixmap("gui/icon/"+name) );

    --player(pl+1).units[name];
    player(pl+1).setGold( player(pl+1).gold() +
                          (75*game.prototype(name).data.gold)/100 );

    //debug    
    if( isTestRun )
      --player(2).units[name];
    } else
  if( ch=='g' && !player(pl+1).isInQueue(name.data()) ){
    if( name=="house" && player(pl+1).economyGrade>=4 ){
      GameMessages::message( L"$(desertstrike/house_cap)",
                             res.pixmap("gui/icon/cancel") );
      } else
    if( name=="castle" && player(pl+1).castleGrade>=2 ){
      GameMessages::message( L"$(desertstrike/house_cap)",
                             res.pixmap("gui/icon/cancel") );
      } else
    if( player(pl+1).gold() < game.prototype(name).data.gold ){
      GameMessages::message( L"$(message/no_gold)",
                             res.pixmap("gui/icon/gold") );
      } else {
      player(pl+1).addGold( -game.prototype(name).data.gold );

      DPlayer::QElement m;
      m.name = name;
      player(pl+1).queue.push_back(m);
      grade( player(pl+1), player(pl+1).queue.back() );
      }
    }else
  if( ch=='m' ){
    //spellToCast = "spellToCast";
    //game.curWorld().emitHudAnim();
    }

  mmap->updateValues();
  }

void DesertStrikeScenario::grade( DPlayer &pl,
                                  DPlayer::QElement &e ){
  const std::string & grade = e.name;

  if( grade=="house" )
    e.maxBTime = 500 + pl.economyGrade*100 +
                       pl.economyGrade*pl.economyGrade*100;

  if( grade=="castle" )
    e.maxBTime = 500 + pl.castleGrade*250;

  e.btime    = e.maxBTime;
  }

Player *DesertStrikeScenario::createPlayer() {
  return new DPlayer( plCount() );
  }

DesertStrikeScenario::DPlayer &DesertStrikeScenario::player(int i) {
  return (DesertStrikeScenario::DPlayer&)Scenario::player(i);
  }

DesertStrikeScenario::DPlayer &DesertStrikeScenario::player() {
  return (DesertStrikeScenario::DPlayer&)Scenario::player();
  }

int DesertStrikeScenario::tierOf(const char *u) {
  for( int i=0; i<3; ++i )
    for( int r=0; r<4; ++r )
      if( units[i][r] && strcmp(units[i][r], u)==0 )
        return i;
  return 0;
  }

void DesertStrikeScenario::onUnitDied(GameObject &obj) {
  if( unitToView==&obj ){
    for( size_t i=0; i<obj.player().unitsCount(); ++i )
      if( &obj.player().unit(i)==unitToView )
        unitToView = 0;
    //hasVTracking = (unitToView!=0);
    }
  if( obj.playerNum()>=1 && obj.playerNum()<=2 )
    --player(obj.playerNum()).realCount[ obj.getClass().name ];

  if( obj.getClass().name=="tower" ){
    if( obj.player().team()==game.player().team() )
      GameMessages::message( L"$(desertstrike/tower_lose)",
                             res.pixmap("gui/icon/tower") );
    else
      GameMessages::message( L"$(desertstrike/tower_kill)",
                             res.pixmap("gui/icon/tower") );
    }
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

  if( hasVTracking ){
    unitToView = 0;

    if( !unitToView ){
      int l = 0;
      for( size_t i=0; i<player().unitsCount(); ++i )
        if( player().unit(i).getClass().data.speed>0 ){
          GameObject &u = player().unit(i);
          int x = u.x()/Terrain::quadSize,
              y = u.y()/Terrain::quadSize;

          int l2 = x*x + y*y;

          if( unitToView==0 || l2>l ){
            unitToView = &player().unit(i);
            l = l2;
            }
          }
      }

    if( unitToView )
      game.setCameraPosSmooth( *unitToView, 0.1 );
    }

  if( tNum%interval==0 ){
    if( !isTestRun ){
      for( int i=1; i<game.plCount(); ++i )
        if( game.player(i).isAi() )
          aiTick(i);
      }
    }

  ++tNum;
  if( tNum/interval>=1 ){
    tNum -= interval;
    //aiTick(2);
    }

  cen->setRemTime( (interval-tNum+Game::ticksPerSecond/2)/Game::ticksPerSecond );

  updateCenterOwner();
  if( plCenter )
    cen->setColor( game.player(plCenter).color() ); else
    cen->setColor( Tempest::Color(0,0,0,0) );

  for( int i=1; i<game.plCount(); ++i ){
    DPlayer &pl = player(i);

    for( size_t r=0; r<pl.queue.size();  ){
      DPlayer::QElement &e = pl.queue[r];

      if( e.btime>0 ){
        mmap->update();
        --e.btime;
        }

      if( e.btime==0 ){
        if( e.maxBTime ){
          e.maxBTime = 0;
          mmap->update();
          }

        if( e.name=="house" ){
          ++pl.economyGrade;

          if( pl.hasHostControl() )
            GameMessages::message( L"$(message/upgrade_complete)",
                                   res.pixmap("gui/icon/house") );
          }

        if( e.name=="castle" ){
          ++pl.castleGrade;

          if( player(i).hasHostControl() )
            GameMessages::message( L"$(message/upgrade_complete)",
                                   res.pixmap("gui/icon/castle") );
          }

        pl.queue[r] = pl.queue.back();
        pl.queue.pop_back();
        } else {
        ++r;
        }
      }

    if( tNum%20==0 ){
      int inc = player(i).team()==player(plCenter).team() ? 5:2;

      bool incH = true;
      for( size_t r=0; r<player(i).queue.size(); ++r )
        if( player(i).queue[r].name=="house" )
          incH = false;

      if( incH )
        player(i).addGold( inc+2*player(i).economyGrade );
      }
    }
  }

void DesertStrikeScenario::cancelTracking( float, float,
                                           Tempest::Event::MouseButton,
                                           MiniMapView::Mode) {
  unitToView   = 0;
  hasVTracking = 0;
  }

void DesertStrikeScenario::mkUnits( int p,
                                    int   x, int   y,
                                    int tgX, int tgY,
                                    bool rev ){
  DPlayer &pl = player(p);

  std::map<std::string, int>::iterator u, e = pl.units.end();
  int count = 0;

  for( u = pl.units.begin(); u!=e; ++u )
    count += u->second;

  int qc = 1;
  //std::max(Math::sqrt(count)-1,1);
  while( qc*qc<count )
    ++qc;

  int id = 0;

  for( u = pl.units.begin(); u!=e; ++u ){
    int& c = pl.realCount[u->first];
    for( int i=0; i<u->second; ++i ){
      if( 1 || c < 3*u->second ){
        GameObject& obj = game.curWorld().addObject(u->first, pl.number());
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
  unitToView   = 0;
  hasVTracking = 1;

  int w = game.curWorld().terrain().width() *Terrain::quadSize;
  int h = game.curWorld().terrain().height()*Terrain::quadSize;

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
    obj.setClass( &game.prototype("tower") );
    obj.setHP( obj.getClass().data.maxHp );
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

void DesertStrikeScenario::updateCenterOwner() {
  bool plCnt[2][2] = {};

  World &wx = game.curWorld();
  for( size_t i=0; i<wx.activeObjects().size(); ++i ){
    GameObject & obj = *wx.activeObjects()[i];
    int w = game.curWorld().terrain().width(),
        h = game.curWorld().terrain().height(),
        x = w*Terrain::quadSize-obj.x(),
        y = obj.y();

    if( obj.player().number()==1 || obj.player().number()==2 ){
      if( x*h > y*w ){
        //obj.setPlayer(1);
        plCnt[0][ obj.player().number()-1 ] = true;
        } else {
        //obj.setPlayer(2);
        plCnt[1][ obj.player().number()-1 ] = true;
        }
      }
    }

  if( plCnt[1][0] && !plCnt[0][1] && plCenter!=1 ){
    GameMessages::message( L"$(desertstrike/center_inc)",
                           res.pixmap("gui/icon/castle") );
    plCenter = 1;
    }

  if( plCnt[0][1] && !plCnt[1][0] && plCenter!=2 ){
    GameMessages::message( L"$(desertstrike/center_dec)",
                           res.pixmap("gui/icon/castle") );
    plCenter = 2;
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

  Widget * top = new Widget();
  cen = new CentralPanel(*this, res);

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

  SpellPanel *s = new SpellPanel(res, game, player());
  s->toogleCameraMode.bind(this, &DesertStrikeScenario::toogleCameraMode );
  cen->layout().add( s );
  cen->layout().add( new GameMessages(res) );

  Tempest::Widget * box = new Tempest::Widget();
  box->layout().add( editPanel );
  box->layout().add( settingsPanel );
  cen->layout().add( box );
  cen->layout().add( new UpgradePanel(res, game, player(1), buyUnitPanel) );

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

  mmap = new Minimap(res,game, player(1));
  mmap->base->renderScene.bind( mainWidget->renderScene );
  mainWidget->updateView.bind( *mmap->base, &UnitView::updateView );
  mmap->base->onClick.bind(*mmap, &Minimap::hideInfo);

  minimap = mmap;
  minimap->setTexture( res.texture("grass/diff") );
  minimap->mouseEvent.bind( mainWidget->minimapEvent );
  minimap->mouseEvent.bind(this, &DesertStrikeScenario::cancelTracking );

  img->setLayout( Vertical );
  img->layout().add( minimap );
  img->layout().setMargin(8);

  console->layout().add( img );
  Widget *w = new Widget();
  w->setSizePolicy( Tempest::Expanding );
  console->layout().add( w );

  buyUnitPanel = new BuyUnitPanel( res, game, player(1), mmap );
  console->layout().add( buyUnitPanel );
  console->useScissor(false);

  return console;
  }

void DesertStrikeScenario::toogleCameraMode() {
  hasVTracking = !hasVTracking;
  unitToView   = 0;
  }

int DesertStrikeScenario::DPlayer::getParam(const std::string &p) const {
  if( p=="atack" )
    return atkGrade;

  if( p=="armor" )
    return armorGrade;

  if( p=="house" )
    return economyGrade;

  if( p=="castle" )
    return castleGrade;

  std::map<std::string, int>::const_iterator i = units.find(p);
  if( i==units.end() )
    return 0;

  return units.at(p);
  }

std::vector<GameObject *> &DesertStrikeScenario::DPlayer::selected() {
  return m.objects;
  }

bool DesertStrikeScenario::DPlayer::isInQueue(const char *ch) {
  for( size_t i=0; i<queue.size(); ++i )
    if( queue[i].name==ch )
      return 1;

  return 0;
  }

void DesertStrikeScenario::aiTick( int npl ) {
  int c = 0;
  DPlayer& pl = player(npl);
  ++pl.aiTick;

  for( auto i=pl.units.begin(); i!=pl.units.end(); ++i )
    c += (i->second);

  if( std::min(4, pl.aiTick/4 + pl.aiTick?1:0 ) > pl.economyGrade ){
    if( !pl.isInQueue("house") &&
        pl.gold() >= game.prototype("house").data.gold ){
      pl.addGold( -game.prototype("house").data.gold );

      DPlayer::QElement el;
      el.name = "house";
      pl.queue.push_back(el);
      grade( pl, el );
      }
    }

  if( !pl.isInQueue("castle") &&
      pl.economyGrade>pl.castleGrade && pl.castleGrade<2  ){
    if( pl.gold() >= game.prototype("castle").data.gold ){
      pl.addGold( -game.prototype("castle").data.gold );

      DPlayer::QElement el;
      el.name = "castle";
      pl.queue.push_back(el);
      grade( pl, el );
      } else {
      return;
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
    for( int i=1; i<3; ++i )
      if( npl!=i ){
        for( int r=count-1; r>=0; --r ){
          const BuildElement& e = b[r];

          int c = player(i).units[e.src];
          c*=e.dcount;
          c*=pass;
          c/=e.scount;

          int tr = tierOf(e.dst.c_str());
          if( tr <= pl.castleGrade &&
              !(pl.castleGrade==2 && tr==0)){
            for( int q=pl.units[e.dst]; q<c; ++q ){
              int gold = game.prototype(e.dst).data.gold;
              if( pl.gold() >= gold ){
                ++pl.units[e.dst];
                pl.setGold( pl.gold() - gold );
                }
              }
            }
          }
        }
    }
  }
