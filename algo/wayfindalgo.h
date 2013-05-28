#ifndef WAYFINDALGO_H
#define WAYFINDALGO_H

#include "util/array2d.h"
#include "algo/algo.h"
#include <memory>

class Terrain;
class GameObject;
class Point;

class WayFindAlgo {
  public:
    // typedef std::shared_ptr<GameObject> PGameObject;
    WayFindAlgo( const Terrain & t );

    void fillClasrerMap( const std::vector<GameObject*> &obj );
    void findWay( std::vector<GameObject*> &obj, int x, int y );

    void findWay( GameObject &obj,
                  int x, int y, int rx, int ry );

    std::vector<Point> way;
  private:
    const Terrain & terrain;

    array2d<int> clasterMap, wayMap;
    int clasterNum;

    std::vector<Point> rwPoint;
    std::vector<Pt>    waveBuf;

    void dump();
    void fill( int x, int y, int v );
    void findWay( std::vector< GameObject* >& objs, GameObject &obj,
                  int x, int y, int rx, int ry, int cls );

    bool isQuadEnable(const array2d<int> &map, Point p, Point src);
    bool isQuadEnable( Point p, Point src );

    void optimizeWay();
    bool optimizeWay( Point a, Point b );

    int rPointX, rPointY;
    bool isRPoint();

    void setupWaysGroup( GameObject& robj,
                         std::shared_ptr<std::vector<Point> >& way,
                         std::vector< GameObject* >& objs,
                         std::vector< std::shared_ptr< std::vector<Point> > >& w);
  };

#endif // WAYFINDALGO_H
