#include "mainmenu.h"

#include "button.h"
#include "panel.h"
#include <Tempest/Layout>
#include <Tempest/Android>

#include "optionswidget.h"
#include "lang/lang.h"
#include "resource.h"

#include "mapselectmenu.h"

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

MainMenu::MainMenu(Resource &res, Tempest::Widget* owner, bool startM)
  :ModalWindow(res, owner), res(res) {
  using namespace Tempest;

  setLayout( Vertical );

  Widget *m = new Panel(res);

  m->setMaximumSize(200, 300);
  m->setMinimumSize(200, 300);
  m->setSizePolicy( Tempest::FixedMin );
  m->setSpacing(16);
  m->setMargin(25);

  m->setLayout( Vertical );

  if( startM )
    m->layout().add( button(res, Lang::tr("$(game_menu/play)"), &MainMenu::start) );

  if( !startM )
    m->layout().add( button(res, Lang::tr("$(game_menu/continue_game)"), &MainMenu::continueGame) );

  m->layout().add( button(res, Lang::tr("$(game_menu/tutorial)"), &MainMenu::start) );

  m->layout().add( button(res, Lang::tr("$(game_menu/options)"), &MainMenu::showOptions) );
  m->layout().add( button(res, Lang::tr("$(game_menu/rate)"),    &MainMenu::rate) );

  layout().add( new Tempest::Widget() );
  layout().add(m);
  layout().add( new Tempest::Widget() );
  showAds(true);
  }

MainMenu::~MainMenu() {
  showAds(false);
  }

Button *MainMenu::button( Resource &res, const std::wstring& s,
                          void (MainMenu::*f)() ) {
  Button *b = new Btn(res);
  b->setText(s);
  b->setMaximumSize( Tempest::Size(250, 50) );
  b->setFont( Tempest::Font(20) );
  b->setSizePolicy( Tempest::Preferred, Tempest::FixedMax );
  b->clicked.bind(this, f);
  //b->setMaximumSize( Tempest::SizePolicy::maxWidgetSize() );

  return b;
  }

void MainMenu::startMap(const std::string &) {

  deleteLater();
  }

void MainMenu::start() {
  MapSelectMenu *m = new MapSelectMenu(res, this);
  m->acepted.bind(this, &MainMenu::startMap);
  }

void MainMenu::continueGame() {
  deleteLater();
  }

void MainMenu::showOptions() {
  new OptionsWidget(res, this);
  }

void MainMenu::rate() {
#ifdef __ANDROID__
  JNIEnv *env       = Tempest::AndroidAPI::jenvi();
  jclass clazz      = env->FindClass( "com/tempest/game/GameActivity" );
  jmethodID rate = env->GetStaticMethodID( clazz, "rateGame", "()V");

  env->CallStaticVoidMethod(clazz, rate);
#endif
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
