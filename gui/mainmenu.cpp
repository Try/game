#include "mainmenu.h"

#include "button.h"
#include "panel.h"
#include <Tempest/Layout>

#include "optionswidget.h"

MainMenu::MainMenu(Resource &res, Tempest::Widget* owner)
  :ModalWindow(res, owner), res(res) {
  using namespace Tempest;


  setLayout( Vertical );

  Widget *m = new Panel(res);

  m->setMaximumSize(500, 500);
  m->setMinimumSize(500, 500);
  m->setSizePolicy( Tempest::FixedMin );
  m->setSpacing(16);
  m->setMargin(25);

  m->setLayout( Vertical );
  Widget *w = new Widget();
  w->setLayout( Horizontal );

  w->layout().add( button(res, L"Start"   , &MainMenu::start) );
  w->layout().add( button(res, L"Tutorial", &MainMenu::start) );
  m->layout().add(w);

  w = new Widget();
  w->setLayout( Horizontal );

  w->layout().add( button(res, L"Options", &MainMenu::showOptions) );
  w->layout().add( button(res, L"Rate"   , &MainMenu::start) );
  m->layout().add(w);

  layout().add( new Tempest::Widget() );
  layout().add(m);
  layout().add( new Tempest::Widget() );
  }

MainMenu::~MainMenu() {

  }

Button *MainMenu::button( Resource &res, const std::wstring& s,
                          void (MainMenu::*f)() ) {
  Button *b = new Button(res);
  b->setText(s);
  b->setMaximumSize( Tempest::Size(250, 250) );
  b->setFont( Tempest::Font(50) );
  b->setSizePolicy( Tempest::Preferred );
  b->clicked.bind(this, f);
  //b->setMaximumSize( Tempest::SizePolicy::maxWidgetSize() );

  return b;
  }

void MainMenu::start() {
  deleteLater();
  }

void MainMenu::showOptions() {
  OptionsWidget *m = new OptionsWidget(res, this);
  //m->onClosed.bind( game, &Game::unsetPause );
  }

void MainMenu::showAds(bool s) {

  }
