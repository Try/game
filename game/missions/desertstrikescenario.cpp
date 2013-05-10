#include "desertstrikescenario.h"

#include "game.h"

DesertStrikeScenario::DesertStrikeScenario(Game &game, MainGui &ui):game(game), ui(ui) {
  tNum     = 0;
  interval = 1000;
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
