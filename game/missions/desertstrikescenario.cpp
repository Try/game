#include "desertstrikescenario.h"

#include "desertstrikescenariowidgets.h"

#include "game.h"
#include "lang/lang.h"

#include "gui/gen/ui_toppanel.h"
#include "gui/missiontargets.h"
#include "gui/unitview.h"
#include "gui/gamemessages.h"
#include "util/weakworldptr.h"

#include "util/bytearrayserialize.h"

#include "util/math.h"
#include "algo/algo.h"
#include "gui/mainmenu.h"

#include <cmath>

const char * DesertStrikeScenario::units[3][4] = {
  {"gelion", "pikeman", "incvisitor", "water_mage"},
  {"fire_mage", "balista"},
  {"fire_element", "golem"}
  };

struct DesertStrikeScenario::GoldButton : public Button {
  GoldButton( Resource &res, Game& g ):Button(res), game(g){}

  void paintEvent(Tempest::PaintEvent &e){
    Button::paintEvent(e);
    Tempest::Painter p(e);
    p.setBlendMode( Tempest::alphaBlend );
    int ws = w()-6;

    DPlayer &pl = (DPlayer&)game.player();

    for( size_t i=0; i<pl.queue.size(); ++i ){
      if( pl.queue[i].name=="house" ){
        p.setColor(1,1,0, 0.3);
        int coolDown = ws*pl.queue[i].btime/pl.queue[i].maxBTime;
        p.drawRect(3,3, coolDown,h()-6);
        }

      if( pl.queue[i].name=="castle" ){
        p.setColor(0,0,1, 0.3);
        int coolDown = ws*pl.queue[i].btime/pl.queue[i].maxBTime;
        p.drawRect(3,3, coolDown,h()-6);
        }
      }
    }

  Game &game;
  };

bool DesertStrikeScenario::defaultMainMenu = 1;

DesertStrikeScenario::DesertStrikeScenario(Game &g, MainGui &ui, BehaviorMSGQueue &msg)
  :Scenario(g,ui, msg){
  tNum      = 0;
  interval  = 25*Game::ticksPerSecond;
  isTestRun = 0;
  plCenter  = 0;

  cameraSpeed = 0;

  unitToView   = 0;
  hasVTracking = 1;

  isMouseTracking = false;

  player(1).units["pikeman"] = 1;
  player(2).units["pikeman"] = 1;

  if( !isTestRun ){
    player(1).setGold(250);
    player(2).setGold(250);
    }

  gui.update();
  g.prototypes().load("campagin/td.json");

  sctrl.isScaleMode = false;

  winAnim.isWinAnim = false;  
  }

DesertStrikeScenario::~DesertStrikeScenario() {
  game.prototypes().unload();
  }

void DesertStrikeScenario::mouseDownEvent( Tempest::MouseEvent &e ) {
  mpos3d          = unProjectRz( e.x, e.y, moveZ );
  isMouseTracking = true;

  mpressPos = e.pos();

  if( e.mouseID==0 ){
    sctrl.m0  = e.pos();
    sctrl.rm0 = sctrl.m0;
    }

  if( e.mouseID==1 ){
    sctrl.m1 = e.pos();
    sctrl.rm1 = sctrl.m1;
    isMouseTracking = false;

    sctrl.mouseD0     = (sctrl.m0 - sctrl.m1).manhattanLength();
    sctrl.d0          = game.curWorld().camera.distance();
    sctrl.isScaleMode = (sctrl.mouseD0>5);
    sctrl.angle0      = game.curWorld().camera.spinX();
    }
  }

void DesertStrikeScenario::mouseUpEvent( Tempest::MouseEvent & e ) {
  if( e.mouseID==1 ){
    sctrl.m1          = e.pos();
    sctrl.isScaleMode = false;
    isMouseTracking = false;
    }

  if( e.mouseID==0 && !sctrl.isScaleMode){
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

    if( spellToCast.size() ){
      if( e.button==Tempest::MouseEvent::ButtonLeft ){
        if( spellMode==Spell::CastToCoord ){
          game.message( game.player().number(),
                        BehaviorMSGQueue::SpellCast,
                        game.curWorld().mouseX(),
                        game.curWorld().mouseY(),
                        spellToCast
                        );
          }

        if( spellMode==Spell::CastToUnit && game.curWorld().mouseObj() ){
          WeakWorldPtr p = game.curWorld().objectWPtr( game.curWorld().mouseObj() );

          game.message( game.player().number(),
                        BehaviorMSGQueue::SpellCast,
                        p.id(),
                        spellToCast
                        );
          }
        }
      } else {
      world.clickEvent( World::coordCastD(v.data[0]),
                        World::coordCastD(v.data[1]),
                        e );
      }
    }
  }

void DesertStrikeScenario::mouseMoveEvent( Tempest::MouseEvent &e ) {
  if( !isMouseTracking && gui.mouseMoveEvent(e) ){
    acceptMouseObj = false;
    return;
    }

  if( e.mouseID==0 && sctrl.isScaleMode ){
    mpos3d = unProject( e.x, e.y, moveZ );
    sctrl.rm0 = e.pos();
    }

  if( e.mouseID==1 && sctrl.isScaleMode ){
    sctrl.rm1 = e.pos();
    double lm = Math::sqrt( (sctrl.m0 - e.pos() ).quadLength() );

    double a0 = atan2( sctrl.m0.y - sctrl.m1.y,
                       sctrl.m0.x - sctrl.m1.x ),
           a1 = atan2( sctrl.rm0.y - sctrl.rm1.y,
                       sctrl.rm0.x - sctrl.rm1.x );

    World &w = game.curWorld();

    w.camera.setSpinX( sctrl.angle0+(a1-a0)*180/M_PI );
    if( lm>5 ){
      w.camera.setDistance( sctrl.d0 * (sctrl.mouseD0/lm) );
      }
    }

  if( sctrl.isScaleMode )
    return;

  acceptMouseObj = true;

  if( isMouseTracking ){
    F3 m = unProject( e.x, e.y, moveZ );
    game.curWorld().moveCamera( mpos3d.data[0]-m.data[0],
                                mpos3d.data[1]-m.data[1],
                                false );
    mpos3d = unProject( e.x, e.y, moveZ );

    Tempest::Point p = (e.pos()-mpressPos);
    if( p.x*p.x + p.y*p.y > 5*5 ){
      unitToView   = 0;
      hasVTracking = 0;
      }
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

        onUnitHired(name);
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
    if( spellToCast==name ){
      spellToCast = "";
      } else {
      spellToCast = name;
      spellMode   = game.prototypes().spell( name ).mode;
      }
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
    }

  if( obj.playerNum()>=1 && obj.playerNum()<=2 )
    --player(obj.playerNum()).realCount[ obj.getClass().name ];

  if( obj.playerNum() ){
    if( obj.getClass().name=="tower" ){
      if( obj.player().team()==game.player().team() )
        GameMessages::message( L"$(desertstrike/tower_lose)",
                               res.pixmap("gui/icon/tower") );
      else
        GameMessages::message( L"$(desertstrike/tower_kill)",
                               res.pixmap("gui/icon/tower") );
      }

    if( obj.getClass().name=="castle" && !winAnim.isWinAnim ){
      winAnim.setup( Tempest::Point(obj.x(), obj.y()),
                     obj.team()!=player().team() );
      }
    }
  }

void DesertStrikeScenario::tick() {
  if( winAnim.isWinAnim ){
    double x = World::coordCast(winAnim.viewPos.x),
           y = World::coordCast(winAnim.viewPos.y);
    if( Tempest::Point( game.curWorld().camera.x(),
                        game.curWorld().camera.y() ) != Tempest::Point(x,y) ){
      double k = 0.1;
      game.setCameraPosition( game.curWorld().camera.x()+(x-game.curWorld().camera.x())*k,
                              game.curWorld().camera.y()+(y-game.curWorld().camera.y())*k );
      } else {
      if( winAnim.timer<=0)
        showWinLose(); else
        --winAnim.timer;
      }

    return;
    }

  Tempest::Point p1x = player(1).spawnPoint,
                 p2x = player(2).spawnPoint;

  if( tNum%interval==0 ){
    mkUnits( 1,
             p1x.x, p1x.y,
             p2x.x, p2x.y,
             0 );
    }

  if( tNum%interval==0 ){
    mkUnits( 2,
             p2x.x, p2x.y,
             p1x.x, p1x.y,
             1 );
    }

  if( hasVTracking ){
    //unitToView = 0;

    if( !unitToView || !unitToView->isOnMove() ){
      int l = 0;
      if( unitToView ){
        GameObject &u = *unitToView;
        int x = (u.x() - player(u.playerNum()).spawnPoint.x )/Terrain::quadSize,
            y = (u.y() - player(u.playerNum()).spawnPoint.y )/Terrain::quadSize;

        l = x*x + y*y;
        }

      for( size_t i=0; i<player().unitsCount(); ++i )
        if( player().unit(i).getClass().data.speed>0 ){
          GameObject &u = player().unit(i);
          int x = (u.x() - player(u.playerNum()).spawnPoint.x )/Terrain::quadSize,
              y = (u.y() - player(u.playerNum()).spawnPoint.y )/Terrain::quadSize;

          int l2 = x*x + y*y;
          if( unitToView==0 || l2>l ){
            unitToView = &player().unit(i);
            l = l2;
            }
          }
      }

    if( unitToView ){
      float x = World::coordCast(unitToView->x()),
            y = World::coordCast(unitToView->y());

      float dx = -(game.curWorld().camera.x() - x ),
            dy = -(game.curWorld().camera.y() - y );
      float l = sqrt(dx*dx+dy*dy);

      cameraSpeed = std::min(cameraSpeed, l);

      float v = cameraSpeed*(l);

      if( l ){
        dx = dx*std::min(1.0f, v/l);
        dy = dy*std::min(1.0f, v/l);
        cameraSpeed += 0.005;

        game.setCameraPosition( game.curWorld().camera.x()+dx,
                                game.curWorld().camera.y()+dy );
        } else {
        cameraSpeed = 0;
        }
      //game.setCameraPosSmooth( *unitToView, 0.1 );
      }
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
      int inc = player(i).team()==player(plCenter).team() ? 5:3;

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

  revPlPos = rand()%2;

  World &wx = game.curWorld();


  std::vector<Tempest::Point> casP;
  for( size_t i=0; i<wx.activeObjects().size(); ++i )
    if( wx.activeObjects()[i]->getClass().name=="castle" )
      casP.emplace_back( wx.activeObjects()[i]->x()/Terrain::quadSize,
                         wx.activeObjects()[i]->y()/Terrain::quadSize );

  if( casP.size()!=2 )
    return;
  {
    Tempest::Point pt = (casP[0]+casP[1])/2;
    game.setCameraPosition( World::coordCast(pt.x*Terrain::quadSize),
                            World::coordCast(pt.y*Terrain::quadSize) );
  }

  std::vector<GameObject*> aobj;
  aobj.resize( wx.activeObjects().size() );
  for( size_t i=0; i<wx.activeObjects().size(); ++i )
    aobj[i] = wx.activeObjects()[i].get();

  for( size_t i=0; i<aobj.size(); ++i ){
    int x = aobj[i]->x()/Terrain::quadSize,
        y = aobj[i]->y()/Terrain::quadSize,

        ox = aobj[i]->x(),
        oy = aobj[i]->y();
    const ProtoObject &pr = game.prototype( aobj[i]->getClass().name );

    wx.deleteObject( aobj[i] );

    GameObject &obj = wx.addObject(pr, 0, false );
    obj.setPosition(ox, oy);

    if( (casP[0] - Tempest::Point(x,y) ).quadLength() <
        (casP[1] - Tempest::Point(x,y) ).quadLength() ){
      obj.setPlayer(1);
      } else {
      obj.setPlayer(2);
      }
    //obj.setClass( &game.prototype( obj.getClass().name ) );
    obj.setHP( obj.getClass().data.maxHp );

    if( obj.getClass().name=="castle" ){
      player( obj.playerNum() ).spawnPoint = Tempest::Point( obj.x(), obj.y() );
      }
    }
  }

void DesertStrikeScenario::showWinLose() {
  game.pause(1);

  WinLoseScreen *w = new WinLoseScreen(res, mainWidget, game);
  w->isWin = winAnim.isWin;
  }

void DesertStrikeScenario::updateCenterOwner() {
  bool plCnt[2][2] = {};

  Tempest::Point ab = (player(2).spawnPoint - player(1).spawnPoint)/Terrain::quadSize;
  int l = std::max( Math::sqrt(ab.quadLength()), 1 );

  World &wx = game.curWorld();
  for( size_t i=0; i<wx.activeObjects().size(); ++i ){
    GameObject & obj = *wx.activeObjects()[i];

    if( obj.player().number()>=1 ){
      Tempest::Point pos{obj.x(), obj.y()};
      Tempest::Point aq = (pos - player(1).spawnPoint)/Terrain::quadSize;

      int sq = (aq.x*ab.x+aq.y*ab.y);
      int len = sq/l;

      if( (len < l/2) ){
        plCnt[0][ obj.player().number()-1 ] = true;
        }

      if( (len > l/2) ) {
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
  mainWidget->layout().setSpacing(0);

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

  setupTopUi(res,top);

  p.maxSize.h = gold->sizePolicy().maxSize.h;
  top->setSizePolicy(p);

  editPanel     = createEditPanel(mainWidget, res);
  settingsPanel = createSettingsPanel(res);

  cen->setLayout( Tempest::Horizontal );
  cen->layout().add( new MissionTargets(game, res) );

  spellPanel = new SpellPanel(res, game, player());
  spellPanel->toogleCameraMode.bind(this, &DesertStrikeScenario::toogleCameraMode );
  cen->layout().add( spellPanel );
  cen->layout().add( new GameMessages(res) );

  Tempest::Widget * box = new Tempest::Widget();
  box->layout().add( editPanel );
  box->layout().add( settingsPanel );
  cen->layout().add( box );
  UpgradePanel *upnl = new UpgradePanel(res, game, player(1), buyUnitPanel);
  upnl->onPage.bind( this, &DesertStrikeScenario::onPanelChoised );
  cen->layout().add( upnl );
  upgradePanel = upnl;

  cen->useScissor( false );
  box->useScissor( false );

  editPanel->setVisible(0);
  settingsPanel->setVisible(0);

  showEditPanel.activated.bind( *this, &DesertStrikeScenario::toogleEditPanel );
  showSettings. activated.bind( *this, &DesertStrikeScenario::toogleSettingsPanel );
  }

void DesertStrikeScenario::setupTopUi( Resource &res, Tempest::Widget *top ) {
  using namespace Tempest;
  top->setLayout( Horizontal );
  top->layout().add( new Widget() );
  Button *menu = new Button(res);
  menu->clicked.bind( *this, &DesertStrikeScenario::showMenu );
  menu->setShortcut( Tempest::Shortcut(menu, Tempest::KeyEvent::K_ESCAPE) );
  top->layout().add(menu);
  Widget *btns = new Widget();
  btns->setLayout( Horizontal );
  top->layout().add( btns );

  gold = new GoldButton(res, game);
  lim  = new Button(res);
  btns->layout().add( new Widget() );
  btns->layout().add( lim );
  btns->layout().add( gold );

  gold->icon = res.pixmap("gui/coin");
  lim-> icon = res.pixmap("gui/icon/house");

  menu->setText( Lang::tr(L"$(game_menu/menu)") );
  gold->setHint(L"$(gold)");
  lim ->setHint(L"$(units_limit)");
  lim->setVisible(0);

  gold->clicked.bind(this, &DesertStrikeScenario::toogleEditPanel);
  }

void DesertStrikeScenario::toogleEditPanel() {
  editPanel->setVisible( !editPanel->isVisible() );
  }

void DesertStrikeScenario::createMenu( Resource &res, Game &game, Tempest::Widget *w ) {
  //static bool defaultMainMenu = true;
  MainMenu *m = new MainMenu(game, res, w, defaultMainMenu);
  defaultMainMenu = false;

  (void)m;
  //m->onClosed.bind( game, &Game::unsetPause );
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
  mmap->onUnit.    bind( this, &DesertStrikeScenario::onUnitToBuy     );
  mmap->onBuilding.bind( this, &DesertStrikeScenario::onBuildingToBuy );

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

void DesertStrikeScenario::addHigtlight(const Tempest::Rect &rect) {
  HightLight h;
  h.rect  = rect;
  h.alpha = 255;

  hightligt.push_back(h);
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

  bool gradeAccum = false;
  if( std::min(4, pl.aiTick/6 + (pl.aiTick>3?1:0) ) > pl.economyGrade ){
    if( !pl.isInQueue("house") &&
        pl.gold() >= game.prototype("house").data.gold ){
      pl.addGold( -game.prototype("house").data.gold );

      DPlayer::QElement el;
      el.name = "house";
      grade( pl, el );
      pl.queue.push_back(el);
      } else {
      gradeAccum = true;
      }
    }

  if( !pl.isInQueue("castle") &&
      pl.economyGrade>pl.castleGrade && pl.castleGrade<2  ){
    if( pl.gold() >= game.prototype("castle").data.gold ){
      pl.addGold( -game.prototype("castle").data.gold );

      DPlayer::QElement el;
      el.name = "castle";
      grade( pl, el );
      pl.queue.push_back(el);
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

  const char* defaultUnits[] = {
    "fire_element",
    "golem",
    "balista",
    "fire_mage",
    "incvisitor",
    0
    };

  if( !gradeAccum ){
    for( int i=0; defaultUnits[i]; ++i ){
      const char* unit = defaultUnits[i];

      int tr = tierOf(unit);
      if( tr <= pl.castleGrade &&
          !(pl.castleGrade==2 && tr==0)){
        for( int q=pl.units[unit]; q<c; ++q ){
          int gold = game.prototype(unit).data.gold;
          if( pl.gold() >= gold ){
            ++pl.units[unit];
            pl.setGold( pl.gold() - gold );
            }
          }
        }
      }
    }
  }

void DesertStrikeScenario::showMainMenu( bool start ) {
  game.pause(1);

  MainMenu *m = new MainMenu(game, res, mainWidget, start);
  m->onClosed.bind( game, &Game::unsetPause );
  }

void DesertStrikeScenario::WinAnim::setup(const Tempest::Point &p , bool win) {
  timer     = 40;
  isWinAnim = 1;
  viewPos   = p;

  isWin     = win;
  }
