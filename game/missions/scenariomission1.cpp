#include "scenariomission1.h"

#include "game.h"
#include "gui/ingamemenu.h"
#include "gui/panel.h"
#include "gui/unitview.h"

#include "gui/richtext.h"

#include "lang/lang.h"

#include <iostream>

class ScenarioMission1::IntroWidget:public ModalWindow{
  public:
    IntroWidget( Game &game,
                 Resource &res,
                 MyWidget::Widget* ow ):ModalWindow(res, ow){
      Panel *p = new Panel(res);
      p->setMinimumSize(550, 300);
      p->setSizePolicy( MyWidget::FixedMin );

      layout().add( new Widget() );
      layout().add(p);
      layout().add( new Widget() );
      layout().add( new Widget() );

      setLayout( MyWidget::Vertical );

      p->setLayout( MyWidget::Horizontal );
      p->layout().setMargin(8);

      Panel *p2 = new Panel(res);
      p->layout().add( p2 );

      UnitView *v = new UnitView(res);
      v->renderScene.bind( renderScene );
      updateView.bind( *v, &UnitView::updateView );

      v->setupUnit( game, "chest" );
      p->layout().add( v );

      p2->setLayout( MyWidget::Vertical );
      p2->layout().setMargin(16);

      RichText * t = new RichText(res);
      t->setText( Lang::tr(L"$(mission1/intro)") );
      p2->layout().add( t );

      Button * btn = new Button(res);
      btn->setText( Lang::tr(L"$(play)") );
      btn->clicked.bind( *this, &MyWidget::Widget::deleteLater );
      p2->layout().add( btn );
      }

    MyWidget::signal< const MyGL::Scene &,
                      ParticleSystemEngine &,
                      MyGL::Texture2d & > renderScene;
    MyWidget::signal<> updateView;
  };

ScenarioMission1::ScenarioMission1( Game &game,
                                    MainGui & ui )
                 :ui(ui), game(game) {

  }

ScenarioMission1::~ScenarioMission1() {
  delete intro;
  }

void ScenarioMission1::onStartGame() {
  //return;

  game.pause(1);

  intro = new IntroWidget( game,
                           game.resources(),
                           ui.centralWidget() );
  intro->renderScene.bind( ui.renderScene );
  ui.updateView.bind( *this, &ScenarioMission1::updateView );

  intro->onClosed.bind( game,  &Game::unsetPause );
  intro->onClosed.bind( *this, &ScenarioMission1::closeIntro);

  if( game.player().unitsCount() ){
    game.setCameraPos( game.player().unit(0) );
    }
  }

void ScenarioMission1::onItemEvent( GameObject &b ) {
  GameObject & obj = b.world().addObject("skeleton_mage");

  obj.setPosition( b.x(), b.y() - World::coordCastD(3), b.z() );
  obj.setPlayer(3);
  }

void ScenarioMission1::updateView() {
  if( intro )
    intro->updateView();
  }

void ScenarioMission1::closeIntro() {
  intro = 0;
  }
