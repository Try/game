#include "desertstrikescenario.h"

#include "game.h"

DesertStrikeScenario::DesertStrikeScenario(Game &g, MainGui &ui, BehaviorMSGQueue &msg)
  :Scenario(g,ui, msg){
  tNum     = 0;
  interval = 1000;
  isMouseTracking = false;
  }

void DesertStrikeScenario::mouseDownEvent( Tempest::MouseEvent &e ) {
  mpos3d          = unProject( e.x, e.y );
  isMouseTracking = true;
  }

void DesertStrikeScenario::mouseUpEvent( Tempest::MouseEvent & ) {
  isMouseTracking = false;
  }

void DesertStrikeScenario::mouseMoveEvent( Tempest::MouseEvent &e ) {
  if( !isMouseTracking && gui.mouseMoveEvent(e) ){
    acceptMouseObj = false;
    return;
    }

  if( isMouseTracking ){
    F3 m = unProject( e.x, e.y );
    game.curWorld().moveCamera( mpos3d.data[0]-m.data[0],
                                mpos3d.data[1]-m.data[1]);
    mpos3d = unProject( e.x, e.y );
    }
  }

void DesertStrikeScenario::setupUI(InGameControls *mainWidget, Resource &res) {

  }

void DesertStrikeScenario::tick() {
  if( tNum%interval==0 ){
    for( int i=0; i<10; ++i ){
      GameObject& obj = game.curWorld().addObject("pikeman", 1);
      int w = game.curWorld().terrain().width() *Terrain::quadSize;
      int h = game.curWorld().terrain().height()*Terrain::quadSize;

      obj.setPosition( 10*Terrain::quadSize,
                       10*Terrain::quadSize );

      obj.behavior.message( Behavior::MoveSingle, w, h );
      obj.behavior.message( Behavior::AtackMove,  w, h );
      }
    }

  if( tNum%interval==0 ){
    for( int i=0; i<10; ++i ){
      GameObject& obj = game.curWorld().addObject("pikeman", 2);
      int w = game.curWorld().terrain().width() *Terrain::quadSize;
      int h = game.curWorld().terrain().height()*Terrain::quadSize;

      obj.setPosition( w-10*Terrain::quadSize,
                       h-10*Terrain::quadSize );

      obj.behavior.message( Behavior::MoveSingle, 10*Terrain::quadSize, 10*Terrain::quadSize );
      obj.behavior.message( Behavior::AtackMove,  10*Terrain::quadSize, 10*Terrain::quadSize );
      }
    }

  ++tNum;
  if( tNum/interval>=2 ){
    tNum = 0;
    }
  }

void DesertStrikeScenario::onStartGame() {
  {
    GameObject& obj = game.curWorld().addObject("tower", 1);
    int w = game.curWorld().terrain().width() *Terrain::quadSize;
    int h = game.curWorld().terrain().height()*Terrain::quadSize;

    int p = 40-9;
    obj.setPosition( p*w/80, p*h/80 );
    }

  {
    GameObject& obj = game.curWorld().addObject("tower", 2);
    int w = game.curWorld().terrain().width() *Terrain::quadSize;
    int h = game.curWorld().terrain().height()*Terrain::quadSize;

    int p = 40+8;
    obj.setPosition( p*w/80, p*h/80 );
    }
  }
