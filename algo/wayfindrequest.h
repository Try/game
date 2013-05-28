#ifndef WAYFINDREQUEST_H
#define WAYFINDREQUEST_H

#include <map>
#include <vector>
//#include <tuple>
#include "wayfindalgo.h"

class GameObject;
class Terrain;

class WayFindRequest {
  public:
    WayFindRequest( const Terrain &terr );

    void findWay( int x, int y, GameObject* );

    struct Tg{
      int x,y;
      bool operator < ( const Tg & t ) const{
        if( x<t.x )
          return 1;
        if( x>t.x )
          return 0;

        return y<t.y;
        }
      };
    void tick();

  private:
    std::map<Tg, std::vector<GameObject*> > rq;
    void findWay(int x, int y, std::vector<GameObject*>& , const Terrain &t);

    std::vector<bool> visited;
    std::vector<GameObject*> group;    

    const Terrain &terr;
    WayFindAlgo algo;

    void makeGroup(GameObject * obj,
                    std::vector<GameObject *> &objs,
                    std::vector<bool> & visited,
                   std::vector<GameObject *> &gr);

    void findWayGr(int x, int y, std::vector<GameObject*>& , const Terrain &t);
  };

#endif // WAYFINGREQUEST_H
