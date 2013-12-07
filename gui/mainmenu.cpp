#include "mainmenu.h"

#include "button.h"
#include "panel.h"
#include <Tempest/Layout>
#include <Tempest/Android>

#include "optionswidget.h"
#include "lang/lang.h"
#include "resource.h"
#include "game.h"

#include "mapselectmenu.h"
#include "maingui.h"

#include "game/missions/desertstrikescenario.h"
#include "game/missions/desertstriketutorialscenario.h"
#include "game/missions/desertstrikescenariowidgets.h"
#include "game/missions/advanceicscenario.h"

struct MainMenu::Btn : public Button {
  Btn( Resource &res ):Button(res){
    frame   = res.pixmap("gui/hintFrame");
    grad    = res.pixmap("gui/mmenuback");
    back[0] = Tempest::Sprite();
    back[1] = Tempest::Sprite();
    }

  void drawBack(Tempest::Painter &, const Tempest::Rect &){}

  void drawFrame(Tempest::Painter &p, const Tempest::Rect &r){
    Button::drawFrame(p,r);

    p.setTexture(grad);
    int sz = 7;
    p.drawRect( r.x+sz-1, r.y+sz-1, r.w-2*sz, r.h-2*sz,
                0, 0, grad.w(), grad.h() );
    }

  Tempest::Sprite grad;
  };

MainMenu::MainMenu(Game &game, Resource &res, Tempest::Widget* owner, bool startM)
  :ModalWindow(res, owner), res(res), game(game) {
  using namespace Tempest;

  par.reserve(100);

  logo  = res.pixmap("gui/logo");
  efect = res.pixmap("gui/menu_efect");

  static bool firstRun = true;
  fbackgr  = firstRun;
  firstRun = false;

  setLayout( Horizontal );

  Widget *m = new Panel(res);

  m->setMaximumSize(200*MainGui::uiScale, 300*MainGui::uiScale);
  m->setMinimumSize(200*MainGui::uiScale, 300*MainGui::uiScale);
  m->setSizePolicy( Tempest::FixedMin );
  m->setSpacing(16);
  m->setMargin(25);

  m->setLayout( Vertical );

  if( !startM )
    m->layout().add( button(res, Lang::tr("$(game_menu/continue_game)"), &MainMenu::continueGame) );

  //if( startM )
    m->layout().add( button(res, Lang::tr("$(game_menu/play)"), &MainMenu::start) );

  m->layout().add( button(res, Lang::tr("$(game_menu/tutorial)"), &MainMenu::tutorial) );

  m->layout().add( button(res, Lang::tr("$(game_menu/options)"), &MainMenu::showOptions) );
  m->layout().add( button(res, Lang::tr("$(game_menu/rate)"),    &MainMenu::rate) );

  Button *help = new Button(res);
  help->setMinimumSize( DesertStrikeScenario::buttonOptimalSize );
  help->setMaximumSize(help->minSize());
  help->setSizePolicy( FixedMin );
  help->clicked.bind( this, &MainMenu::showHelp );
  help->icon = res.pixmap("gui/info");

  Widget* wl = new Widget();
  wl->setLayout( Vertical );
  wl->layout().add( new Widget() );
  wl->layout().add( help );
  wl->layout().add( new Widget() );

  layout().add(wl);
  layout().add(m);
  layout().add( new Tempest::Widget() );
  showAds(true);

  timer.timeout.bind(this, &MainMenu::update );
  timer.start(1000/60);

  efeTimer.timeout.bind(this, &MainMenu::updateParticles );
  efeTimer.start(1000/20);
  efeTimer.setRepeatCount(16);
  }

MainMenu::~MainMenu() {
  showAds(false);
  }

Button *MainMenu::button( Resource &res, const std::wstring& s,
                          void (MainMenu::*f)() ) {
  Button *b = new Btn(res);
  b->setText(s);
  b->setMaximumSize( Tempest::Size(250*MainGui::uiScale, 50*MainGui::uiScale) );
  b->setFont( Tempest::Font(20) );
  b->setSizePolicy( Tempest::Preferred, Tempest::FixedMax );
  b->clicked.bind(this, f);
  //b->setMaximumSize( Tempest::SizePolicy::maxWidgetSize() );

  return b;
  }

void MainMenu::paintEvent(Tempest::PaintEvent &e) {
  Tempest::Painter p(e);

  if( fbackgr ){
    p.setColor(0,0,0,1);
    p.drawRect(0,0,w(),h());
    p.setColor(1,1,1,1);
    }

  ModalWindow::paintEvent(e);

  p.setBlendMode( Tempest::addBlend );
  p.setTexture( efect );
  for( size_t i=0; i<par.size(); ++i ){
    Particle &px = par[i];
    p.setColor( px.color );
    p.drawRect( Tempest::Rect( px.pos.x-px.sz, px.pos.y-px.sz, 2*px.sz, 2*px.sz ),
                efect.size().toRect() );
    }

  p.setColor(1,1,1,1);
  p.setTexture( logo );
  p.setBlendMode( Tempest::alphaBlend );
  p.drawRect( (w()-logo.w())/2, 50, logo.w(), logo.h() );
  }

void MainMenu::startMap(const std::wstring &m) {
  Game &g = game;

  if( !g.load( m ) )
    return;

  //deleteLater();
  g.setupScenario<DesertStrikeScenario>();
  g.unsetPause();
  }

void MainMenu::startMapExt(const std::wstring &mx) {
  Game &g = game;

  if( !g.load( mx ) )
    return;

  g.setupScenario<AdvanceICScenario>();
  g.unsetPause();
  }

void MainMenu::tutorial() {
  Game &g = game;

  if( !g.load( L"campagin/td1_1.sav" ) )
    return;

  g.setupScenario<DesertStrikeTutorialScenario>();
  g.unsetPause();
  }

void MainMenu::start() {
  MapSelectMenu *m = new MapSelectMenu(res, this);
  m->acepted.bind(this, &MainMenu::startMap);
  m->aceptedExt.bind(this, &MainMenu::startMapExt);
  }

void MainMenu::continueGame() {
  deleteLater();
  game.unsetPause();
  }

void MainMenu::showOptions() {
  new OptionsWidget(res, this);
  }

void MainMenu::rate() {
#ifdef __ANDROID__
  JNIEnv *env       = Tempest::AndroidAPI::jenvi();
  jclass clazz      = env->FindClass( "com/tempest/game/GameActivity" );
  jmethodID rate    = env->GetStaticMethodID( clazz, "rateGame", "()V");

  env->CallStaticVoidMethod(clazz, rate);
#endif
  }

void MainMenu::showHelp() {
  new DesertStrikeScenario::UInfo(res, this, game);
  }

void MainMenu::updateParticles() {
  for( size_t i=0; i<par.size(); ++i ){
    Particle &px = par[i];
    px.pos += px.v;
    px.sz  -= 1;

    if( px.sz<0 )
      px = mkParticle();
    }

  if( par.size() < 100 ){
    par.push_back( mkParticle() );
    }
  }

void MainMenu::closeEvent(Tempest::CloseEvent &e) {
  e.ignore();
  }

void MainMenu::showAds(bool s) {
  (void)s;
#ifdef __ANDROID__
  JNIEnv *env       = Tempest::AndroidAPI::jenvi();
  jclass clazz      = env->FindClass( "com/tempest/game/GameActivity" );
  jmethodID showAds = env->GetStaticMethodID( clazz, "showAds", "(Z)V");

  env->CallStaticVoidMethod(clazz, showAds, s);
#endif
  }

int MainMenu::adsHeight() {
#ifdef __ANDROID__
  JNIEnv *env         = Tempest::AndroidAPI::jenvi();
  jclass clazz        = env->FindClass( "com/tempest/game/GameActivity" );
  jmethodID adsHeight = env->GetStaticMethodID( clazz, "adsHeight", "()I");

  return env->CallStaticIntMethod(clazz, adsHeight);
#endif
  return 0;
  }

MainMenu::Particle MainMenu::mkParticle() {
  Particle p;
  p.pos = Tempest::Point( w()/2 + rand()%41-20,
                          50+rand()%std::max(1,logo.h()) );
  p.v   = Tempest::Point( rand()%7, 0 ) - Tempest::Point(3,0);
  p.sz  = 70+rand()%20;
  p.color.set(1,1,1,1);

  return p;
  }
