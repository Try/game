#include "buildingbehavior.h"

#include "landscape/terrain.h"
#include "game/gameobject.h"
#include "game/world.h"

BuildingBehavior::BuildingBehavior( GameObject &object,
                                    Behavior::Closure & ):obj(object) {
  fill( obj.x()/Terrain::quadSize,
        obj.y()/Terrain::quadSize,
        obj.getClass().data.size, 1 );

  }

BuildingBehavior::~BuildingBehavior() {
  fill( x, y, obj.getClass().data.size, -1 );
  }


bool BuildingBehavior::canBuild( const Terrain &t, const ProtoObject &p,
                                 int x, int y ) {
  int size = p.data.size;

  x = x/Terrain::quadSize - size/2+size%2;
  y = y/Terrain::quadSize - size/2+size%2;

  for( int i=0; i<size; ++i )
    for( int r=0; r<size; ++r )
      if( !t.isEnableForBuilding(i+x, r+y) )
        return 0;

  return 1;
  }

bool BuildingBehavior::message( AbstractBehavior::Message msg,
                                int /*x*/, int /*y*/,
                                AbstractBehavior::Modifers /*md*/ ) {

  if( msg==onPositionChange ){
    fill( x, y, obj.getClass().data.size, -1 );

    fill( obj.x()/Terrain::quadSize,
          obj.y()/Terrain::quadSize,
          obj.getClass().data.size, 1 );
    }

  return 0;
  }

void BuildingBehavior::fill( int ox, int oy, int size, int dv ) {
  x = ox;
  y = oy;
  fill( obj.world().terrain(), ox, oy, size, dv );
  }

void BuildingBehavior::fill( Terrain & terrain,
                             int ox, int oy, int size, int dv ) {
  terrain.editBuildingsMap( ox - size/2+size%2,
                            oy - size/2+size%2,
                            size, size, dv);
  }
