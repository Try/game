#ifndef BUILDINGBEHAVIOR_H
#define BUILDINGBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"

class Point;
class ProtoObject;

class BuildingBehavior : public AbstractBehavior {
  public:
    BuildingBehavior( GameObject & obj,
                      Behavior::Closure & c );
    ~BuildingBehavior();

    bool message( Message msg,
                  int x, int y,
                  Modifers md = NoModifer );
    void tick( const Terrain & /*terrain*/ ){}

    static bool canBuild( const Terrain &t, const ProtoObject &p, int x, int y );
    static void fill( Terrain &t, int ox, int oy, int size, int dv );
  private:
    GameObject & obj;
    int x,y;

    void fill( int ox, int oy, int size, int dv );
  };

#endif // BUILDINGBEHAVIOR_H
