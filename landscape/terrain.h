#ifndef TERRAIN_H
#define TERRAIN_H

#include "util/array2d.h"

#include <MyGL/Model>
#include "resource.h"

#include <memory>

namespace MyGL{
  class VertexBufferHolder;
  class IndexBufferHolder;
  }

class GameObject;
class GameSerializer;

struct WaterVertex: MVertex{
  float h;
  float dir[2];
  };

#include "game/gameobjectview.h"

class Terrain {
  public:
    Terrain( int w, int h,
             MyGL::Scene & s,
             World       & wrld,
             const PrototypesLoader & pl );

    struct EditMode{
      EditMode();

      enum EHeight{
        None,
        Up,
        Down
        };
      EHeight map, wmap;
      double R;

      bool isEnable, isSecondaryTexturing;
      std::string texture;
      };

    void buildGeometry( MyGL::VertexBufferHolder & vboHolder,
                        MyGL::IndexBufferHolder  & iboHolder );
    /*
    Model buildGeometry( MyGL::VertexBufferHolder & vboHolder,
                         MyGL::IndexBufferHolder  & iboHolder) const;
                         */

    MyGL::Model<WaterVertex> waterGeometry( MyGL::VertexBufferHolder & vboHolder,
                                            MyGL::IndexBufferHolder  & iboHolder) const;

    int width() const;
    int height() const;

    double viewWidth() const;
    double viewHeight() const;

    void brushHeight(int x, int y, const EditMode &m, bool alternative);
    int  at( int x, int y ) const;
    int  atF( float x, float y ) const;
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

    void serialize( GameSerializer &s );
  private:
    MyGL::Scene            & scene;
    World                  & world;
    const PrototypesLoader & prototype;

    struct View{
      std::shared_ptr<GameObjectView> view;
      };
    std::vector<View> landView;

    std::vector< std::string > aviableTiles;
    struct Tile {
      int plane;
      size_t textureID[2];
      float normal[3];
      };
    array2d<Tile> tileset;
    std::vector< Model::Vertex > land, minor;

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

    void buildGeometry(MyGL::VertexBufferHolder & vboHolder,
                        MyGL::IndexBufferHolder  & iboHolder,
                        int plane,
                        size_t texture );

    void computePlanes();
  };

#endif // TERRAIN_H
