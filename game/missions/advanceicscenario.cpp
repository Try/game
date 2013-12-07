#include "advanceicscenario.h"

#include "game.h"

AdvanceICScenario::AdvanceICScenario(Game &game, MainGui &ui, BehaviorMSGQueue &msg)
  :DesertStrikeScenario(game, ui, msg){

  }

void AdvanceICScenario::tick() {
  DesertStrikeScenario::tick();

  Tempest::Point px[2] = {
    player(2).spawnPoint,
    player(1).spawnPoint
    };

  if( tNum%(10*Game::ticksPerSecond)==0 ){
    for( int r=1; r<=2; ++r ){
      Tempest::Point tg;
      if( player(r).number()>=1 && player(r).number()<2 )
        tg = px[ player(r).number()-1 ];
      std::vector<Tempest::Point> pos;

      for( size_t i=0; i<player(r).unitsCount(); ++i ){
        GameObject &u = player(r).unit(i);
        const ProtoObject &p = u.getClass();
        if( p.name=="barracks" ){
          pos.push_back(Tempest::Point(u.x(), u.y()));
          }
        }

      for( size_t i=0; i<pos.size(); ++i ){
        Tempest::Point u = pos[i];

        for( int q=0; q<3; ++q )
          spawn( "pikeman", player(r).number(),
                 u.x, u.y,
                 tg.x, tg.y );
        }
      }
    }
  }
