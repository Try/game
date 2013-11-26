#include "scenariomission1.h"

#include "game.h"
#include "gui/ingamemenu.h"
#include "gui/panel.h"
#include "gui/unitview.h"

#include "gui/richtext.h"

#include "lang/lang.h"

#include "util/gameserializer.h"
#include <iostream>

class ScenarioMission1::IntroWidget:public ModalWindow{
  public:
    IntroWidget( Game &game,
                 Resource &res,
                 Tempest::Widget* ow ):ModalWindow(res, ow){
      Panel *p = new Panel(res);
      p->setMinimumSize(550, 300);
      p->setSizePolicy( Tempest::FixedMin );

      layout().add( new Widget() );
      layout().add(p);
      layout().add( new Widget() );
      layout().add( new Widget() );

      setLayout( Tempest::Vertical );

      p->setLayout( Tempest::Horizontal );
      p->layout().setMargin(8);

      Panel *p2 = new Panel(res);
      p->layout().add( p2 );

      UnitView *v = new UnitView(res);
      v->setupUnit( game, "chest" );
      p->layout().add( v );

      p2->setLayout( Tempest::Vertical );
      p2->layout().setMargin(16);

      RichText * t = new RichText(res);
      t->setText( Lang::tr(L"$(mission1/intro)") );
      p2->layout().add( t );

      Button * btn = new Button(res);
      btn->setText( Lang::tr(L"$(game_menu/play)") );
      btn->clicked.bind( *this, &Tempest::Widget::deleteLater );
      p2->layout().add( btn );
      }

    Tempest::signal< const Scene &,
                      ParticleSystemEngine &,
                      Tempest::Texture2d & > renderScene;
    Tempest::signal<> updateView;
  };

class ScenarioMission1::EndScreenWidget:public ModalWindow{
  public:
    EndScreenWidget( Game &game,
                     Resource &res,
                     Tempest::Widget* ow,
                     bool isWin ):ModalWindow(res, ow){
      Panel *p = new Panel(res);
      p->setMinimumSize(300, 200);
      p->setSizePolicy( Tempest::FixedMin );

      layout().add( new Widget() );
      layout().add(p);
      layout().add( new Widget() );
      layout().add( new Widget() );

      setLayout( Tempest::Vertical );

      p->setLayout( Tempest::Vertical );
      p->layout().setMargin(8);

      RichText * t = new RichText(res);
      if( isWin )
        t->setText( Lang::tr(L"$(mission1/win)") ); else
        t->setText( Lang::tr(L"$(game_menu/defeat)") );

      p->layout().add( t );

      Widget *p2 = new Widget();
      p->layout().add(p2);
      p2->setLayout( Tempest::Horizontal );
      p2->setMaximumSize(2000, 30);
      p2->setSizePolicy( Tempest::Preferred, Tempest::FixedMax );

      if( isWin ){
        Button * btn = new Button(res);
        btn->setText( Lang::tr(L"$(game_menu/continue_game)") );
        btn->clicked.bind( *this, &Tempest::Widget::deleteLater );
        p2->layout().add( btn );

        p2->layout().add( new Widget() );
        }

      if( !isWin ){
        Button * btn = new Button(res);
        btn->setText( Lang::tr(L"$(game_menu/restart_mission)") );
        btn->clicked.bind( game.scenario(), &Scenario::restartGame );
        btn->clicked.bind( *this, &Tempest::Widget::deleteLater );
        p2->layout().add( btn );
        p2->layout().add( new Widget() );
        }

      Button * btn = new Button(res);
      btn->setText( Lang::tr(L"$(game_menu/close_game)") );
      btn->clicked.bind( game.exitGame );
      p2->layout().add( btn );
      }

  };

ScenarioMission1::ScenarioMission1( Game &g, MainGui & ui, BehaviorMSGQueue &msg )
                 :Scenario(g,ui,msg) {
  isInGame = true;

  intro    = 0;
  lChest   = 0;
  cutScene.active = false;
  mtagets.clear();

  ui.updateView.bind( *this, &ScenarioMission1::updateView );
  }

ScenarioMission1::~ScenarioMission1() {
  gui.updateView.ubind( *this, &ScenarioMission1::updateView );
  delete intro;
  }

void ScenarioMission1::onStartGame() {
  //return;
  start();
  }

void ScenarioMission1::onItemEvent( GameObject &b ) {
  if( b.getClass().name=="chest" ){
    game.player().setGold( 100500 );
    GameObject & obj = b.world().addObject("skeleton_mage");

    obj.setPosition( b.x(), b.y() - World::coordCastD(4), b.z() );
    obj.setPlayer(5);
    obj.setViewDirection(0, 100);

    for( size_t i=0; i<3; ++i ){
      GameObject & obj2 = b.world().addObject("incvisitor_t");

      obj2.setPosition( obj.x(), obj.y()+Terrain::quadSize*2, obj.z() );
      obj2.setPlayer( obj.playerNum() );
      }

    {
      GameObject & obj2 = b.world().addObjectEnv("ore_sparks");

      obj2.setPosition( b.x(), b.y(), b.z()+500 );
      }

    mtagets[0].done = true;

    MissionTaget tg;
    tg.hint = L"$(mission1/target1)";
    tg.done = 0;

    mtagets.push_back(tg);

    cutScene.active = true;
    cutScene.t      = 0;
    cutScene.camera = b.world().camera;

    float x1 = World::coordCast(b.x()),
          y1 = World::coordCast(b.y()),
          z1 = World::coordCast(b.z());

    cutScene.camera.setPosition(x1, y1, z1);
    gui.setCutsceneMode(1);
    }

  if( b.getClass().name=="chest_small" ){
    if( lChest==0 ){
      MissionTaget tg;
      tg.hint = L"$(mission1/target2)";
      tg.done = 0;

      mtagets.push_back(tg);
      }

    const char* objs[] = {
      "golem",
      "golem_t",
      "bear",
      "fire_element"
      };

    if( lChest<4 ){
      GameObject & obj = b.world().addObject( objs[lChest] );
      obj.setPosition( b.x(), b.y(), b.z() );
      obj.setPlayer(1);
      }

    ++lChest;

    if( lChest==4){
      for( size_t i=0; i<mtagets.size(); ++i )
        if( mtagets[i].hint==L"$(mission1/target2)" )
          mtagets[i].done = true;
      }
    }

  game.updateMissionTargets();
  }

void ScenarioMission1::tick() {
  if( cutScene.active ){
    cutScene.t += 0.01;

    for( size_t i=0; i<game.player(5).unitsCount(); ++i ){
      GameObject & obj = game.player(5).unit(i);

      if( obj.getClass().name=="skeleton_mage" ){
        float x1 = World::coordCast(obj.x()),
              y1 = World::coordCast(obj.y()),
              z1 = World::coordCast(obj.z())+0.4,
              t  = std::min(1.0f, cutScene.t);

        x1 = cutScene.camera.x()*(1-t) + t*x1;
        y1 = cutScene.camera.y()*(1-t) + t*y1;
        z1 = cutScene.camera.z()*(1-t) + t*z1;

        obj.world().camera.setPosition(x1, y1, z1);
        obj.world().camera.setSpinY( cutScene.camera.spinY()*(1-t) + (-110)*t );

        obj.world().camera.setDistance( cutScene.camera.distance()*(1-t) + (1.5)*t );
        }
      }

    if( cutScene.t>=1.3 ){
      cutScene.active = false;
      if( game.player().unitsCount() )
        game.player().unit(0).world().camera = cutScene.camera;

      gui.setCutsceneMode(0);
      }

    return;
    }

  if( game.player(1).unitsCount()==0 && isInGame ){
    defeat();
    isInGame = false;
    }

  for( size_t i=0; i<game.player(5).unitsCount(); ++i )
    if( game.player(5).unit(i).getClass().name=="skeleton_mage" ){
      while( game.player(5).unitsCount()<4 ){
        GameObject & obj  = game.player(5).unit(i);
        GameObject & obj2 = obj.world().addObject("incvisitor_t");

        obj.world().emitHudAnim( "hud/blink",
                                 obj.x(),
                                 obj.y(),
                                 0.01 );

        obj2.setPosition( obj.x(), obj.y(), obj.z() );
        obj2.setPlayer( obj.playerNum() );
        }      
      }

  if( mtagets[0].done && isInGame ){
    bool ok = true;
    for( size_t i=0; i<game.player(5).unitsCount(); ++i )
      if(game.player(5).unit(i).getClass().name=="skeleton_mage" )
        ok = false;

    if( ok ){
      winGame();
      isInGame = false;
      }
    }
  }

const std::vector<Scenario::MissionTaget> &ScenarioMission1::tagets() {
  return mtagets;
  }

void ScenarioMission1::updateView() {
  if( intro )
    intro->updateView();
  }

void ScenarioMission1::closeIntro() {
  intro = 0;
  }

void ScenarioMission1::start() {
  // ui.setCutsceneMode(1);
  game.player().setGold(0);

  isInGame = true;
  lChest   = 0;
  cutScene.active = false;
  mtagets.clear();

  game.pause(1);

  intro = new IntroWidget( game,
                           game.resources(),
                           gui.centralWidget() );
  intro->renderScene.bind( gui.renderScene );

  intro->onClosed.bind( game,  &Game::unsetPause );
  intro->onClosed.bind( *this, &ScenarioMission1::closeIntro);

  if( game.player().unitsCount() ){
    game.setCameraPos( game.player().unit(0) );
    }

  MissionTaget tg;
  tg.hint = L"$(mission1/target0)";
  tg.done = 0;

  mtagets.push_back(tg);

  game.updateMissionTargets();
  }

void ScenarioMission1::defeat() {
  new EndScreenWidget( game,
                       game.resources(),
                       gui.centralWidget(),
                       false );
  }

void ScenarioMission1::restartGame() {
  Game &game = this->game;

  game.load(L"./campagin/0.sav");
  game.scenario().onStartGame();
  }

void ScenarioMission1::winGame() {
  for( size_t i=0; i<game.player(5).unitsCount(); ++i ){
    game.player(5).unit(i).setHP(0);
    }

  for( size_t i=0; i<mtagets.size(); ++i )
    if( mtagets[i].hint==L"$(mission1/target1)" )
      mtagets[i].done = true;

  game.updateMissionTargets();


  new EndScreenWidget( game,
                       game.resources(),
                       gui.centralWidget(),
                       true );
  }

void ScenarioMission1::serialize( GameSerializer &s ) {
  if( s.version()<6 )
    return;

  size_t sz = mtagets.size();
  s + sz;

  if( s.isReader() ){
    mtagets.resize( sz );
    }

  for( size_t i=0; i<mtagets.size(); ++i ){
    s + mtagets[i].hint;
    s + mtagets[i].done;
    }

  s + isInGame + lChest;

  game.updateMissionTargets();
  }
