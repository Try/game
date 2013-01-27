#include "unitview.h"

#include "resource.h"
#include "game/gameobject.h"
#include "game.h"

UnitView::UnitView( Resource &res )
  : TextureView(res), res(res) {
  //texture = res.texHolder.create(120, 200);

  scene.lights().direction().resize(1);
  MyGL::DirectionLight light;
  light.setDirection( -2, 1, -2.0 );
  light.setColor    ( MyGL::Color( 0.7, 0.7, 0.7 ) );
  light.setAblimient( MyGL::Color( 0.33, 0.33,  0.35) );
  scene.lights().direction()[0] = light;

  folowMode = 0;
  curUnit   = 0;

  rotateMode = true;
  rotAngle   = 0;
  }

UnitView::~UnitView() {
  }

void UnitView::setupUnit(GameObject *obj) {
  curUnit = obj;
  view.reset(0);

  if( obj ) {
    setupUnit( obj->game(),
               obj->getClass().name );
    }
  }

void UnitView::setupUnit( Game &game,
                          const std::string & proto ) {
  const PrototypesLoader & prototypes = game.prototypes();

  if( !world ){
    world.reset( new World(game, 1, 1) );
    }

  if( !pEng ){
    pEng.reset( new ParticleSystemEngine(scene, prototypes, res) );
    pEng->setupMaterial.bind( game, &Game::setupMaterials );
    }

  const ProtoObject & p = game.prototype( proto );

  view.reset( new GameObjectView( scene,
                                  *world,
                                  *pEng,
                                  p,
                                  prototypes ) );
  view->loadView( res, world->physics, 0 );
  view->teamColor = MyGL::Color(1, 1, 0, 1);

  view->setViewPosition(0,0,0);

  setupCamera();
  }

void UnitView::updateView() {
  if( curUnit && folowMode ){
    setCameraPos(*curUnit);
    }

  if( texture.width()!=w() || texture.height()!=h() ){
    texture = res.texHolder.create( w(), h() );
    }

  if( pEng ){
    setupCamera();
    renderScene( scene, *pEng, texture );
    }
  }

void UnitView::mouseDownEvent(MyWidget::MouseEvent &) {
  folowMode = (1);
  }

void UnitView::mouseUpEvent(MyWidget::MouseEvent &) {
  folowMode = (0);
  }

void UnitView::paintEvent(MyWidget::PaintEvent &e) {
  //updateView();
  TextureView::paintEvent(e);
  }

void UnitView::setupCamera() {
  MyGL::Camera camera;
  camera.setPerespective( true, w(), h() );
  camera.setPosition( 0, 0, 0 );
  camera.setDistance( 2 );

  bool mv = 0;
  if( view ){
    for( size_t i=0; i<view->getClass().behaviors.size(); ++i )
      if( view->getClass().behaviors[i]=="move" )
        mv = 1;

    camera.setPosition( 0, 0, 2*view->radius() );
    }

  if( rotateMode ){
    rotAngle += 1;
    }

  if( mv )
    camera.setSpinX(-15-180-90+rotAngle); else
    camera.setSpinX(-15-180+rotAngle);

  camera.setSpinY(-130);

  if( view )
    camera.setZoom( 0.7/view->radius() );

  scene.setCamera( camera );
  }
