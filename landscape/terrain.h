#ifndef TERRAIN_H
#define TERRAIN_H

#include "util/array2d.h"

#include <MyGL/Model>

namespace MyGL{
  class VertexBufferHolder;
  class IndexBufferHolder;
  }

class GameObject;

class Terrain {
  public:
    Terrain( int w = 128, int h = 128 );

    MyGL::Model<> buildGeometry( MyGL::VertexBufferHolder & vboHolder,
                                 MyGL::IndexBufferHolder  & iboHolder) const;

    struct WVertex: MyGL::DefaultVertex{
      float h;
      float dir[2];
      };
    MyGL::Model<WVertex> waterGeometry( MyGL::VertexBufferHolder & vboHolder,
                                        MyGL::IndexBufferHolder  & iboHolder) const;

    int width() const;
    int height() const;

    double viewWidth() const;
    double viewHeight() const;

    void brushHeight( int x, int y, double dh, double r );
    int  at( int x, int y ) const;
    int  heightAt( int x, int y ) const;
    int  heightAt( float x, float y ) const;

    bool isEnableW( int x, int y ) const;
    bool isEnable( int x, int y ) const;
    bool isEnableForBuilding( int x, int y ) const;
    bool isEnableQuad( int x, int y, int size = 1 ) const;

    void resetBusyMap();
    // int& busyAt( int x, int y );
    void incBusyAt( int x, int y, GameObject & owner );
    int  busyAt( int x, int y, int sz ) const;
    GameObject * unitAt( int x, int y ) const;

    std::pair<int,int> nearestEnable( int x, int y, int sz ) const;
    std::pair<int,int> nearestEnable( int x, int y,
                                      int tgX, int tgY, int sz ) const;

    static const int   quadSize  = 150;
    static const float quadSizef = quadSize;

    unsigned nextGroupMask() const;

    void editBuildingsMap( int x, int y, int w, int h, int dv );
  private:
    array2d<int>  heightMap;
    array2d<int>  waterMap;
    array2d<int>  buildingsMap;
    array2d<unsigned char>  enableMap;

    mutable int groupMask;

    static const int busyMapsCount;
    struct BusyMap{
      array2d<int>  count;
      array2d<GameObject*>  owner;

      void resize( int w, int h );
      void reset();
      } busyMap[4];

    int clampX( int x ) const;
    int clampY( int y ) const;

    void computeEnableMap();

    int heightAtNoDepth( int x, int y ) const;
    int  depthAt( int x, int y ) const;
  };

#endif // TERRAIN_H
