#ifndef WAYFINDREQUEST_H
#define WAYFINDREQUEST_H

#include <map>
#include <vector>
#include <tuple>

class GameObject;
class Terrain;

class WayFindRequest {
  public:
    WayFindRequest();

    void findWay( int x, int y, GameObject* );

    struct Tg{
      int x,y;
      bool operator < ( const Tg & t ) const{
        return std::tie(x,y) < std::tie(t.x, t.y);
        }
      };
    void tick( const Terrain& t );

  private:
    std::map<Tg, std::vector<GameObject*> > rq;
    void findWay(int x, int y, std::vector<GameObject*>& , const Terrain &t);

    std::vector<bool> visited;
    std::vector<GameObject*> group;

    void makeGroup(GameObject * obj,
                    std::vector<GameObject *> &objs,
                    std::vector<bool> & visited,
                   std::vector<GameObject *> &gr);

    void findWayGr(int x, int y, std::vector<GameObject*>& , const Terrain &t);
  };

#endif // WAYFINGREQUEST_H
