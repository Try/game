#include "scenariomission1.h"

#include "game.h"
#include "gui/ingamemenu.h"
#include "gui/panel.h"
#include "gui/unitview.h"

#include <iostream>

class ScenarioMission1::IntroWidget:public ModalWindow{
  public:
    IntroWidget( Game &game,
                 Resource &res,
                 MyWidget::Widget* ow ):ModalWindow(res, ow){
      Panel *p = new Panel(res);
      p->setMinimumSize(400, 300);
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

      v->setupUnit( game, "fire_mage" );
      p->layout().add( v );

      Button * btn = new Button(res);
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

void ScenarioMission1::updateView() {
  if( intro )
    intro->updateView();
  }

void ScenarioMission1::closeIntro() {
  intro = 0;
  }
