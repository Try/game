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
class SmallGraphicsObject;

struct WaterVertex: MVertex{
  float h;
  float dir[2];
  };

#include "game/gameobjectview.h"
#include "terrainchunk.h"

class Terrain {
  public:
    Terrain( int w, int h,
             Resource & res,
             MyGL::Scene & s,
             World       & wrld,
             const PrototypesLoader & pl );

    struct EditMode{
      EditMode();

      enum EHeight{
        None,
        Up,
        Down,
        Align
        };
      EHeight map, wmap;
      double R;

      bool isEnable, isSecondaryTexturing;
      std::string texture;
      };

    void buildGeometry( MyGL::VertexBufferHolder & vboHolder,
                        MyGL::IndexBufferHolder  & iboHolder );

    MyGL::Model<WaterVertex> waterGeometry( MyGL::VertexBufferHolder & vboHolder,
                                            MyGL::IndexBufferHolder  & iboHolder) const;

    Model fogGeometry( MyGL::VertexBufferHolder & vboHolder,
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

    // int& busyAt( int x, int y );

    std::pair<int,int> nearestEnable( int x, int y, int sz ) const;
    std::pair<int,int> nearestEnable( int x, int y,
                                      int tgX, int tgY, int sz ) const;

    static const int   quadSize  = 600;
    static const float quadSizef = quadSize;

    void editBuildingsMap( int x, int y, int w, int h, int dv );

    void serialize( GameSerializer &s );

    void updatePolish();
  private:
    MyGL::Scene            & scene;
    World                  & world;
    const PrototypesLoader & prototype;

    TerrainChunk::View waterView, fogView;

    std::vector< std::string > aviableTiles;
    struct Tile {
      int plane;
      size_t textureID[2];
      float normal[3];
      };
    array2d<Tile> tileset;
    array2d<TerrainChunk> chunks;

    std::vector< Model::Vertex > land, minor;

    array2d<int>  heightMap;
    array2d<int>  waterMap;
    array2d<int>  buildingsMap;
    array2d<unsigned char>  enableMap;

    int clampX( int x ) const;
    int clampY( int y ) const;

    void computeEnableMap();

    int heightAtNoDepth( int x, int y ) const;
    int  depthAt( int x, int y ) const;

    void buildGeometry( MyGL::VertexBufferHolder & vboHolder,
                        MyGL::IndexBufferHolder  & iboHolder,
                        int plane,
                        size_t texture );

    void buildGeometry( MyGL::VertexBufferHolder & vboHolder,
                        MyGL::IndexBufferHolder  & iboHolder,
                        int plane,
                        size_t texture,
                        int cX, int cy );

    void computePlanes();
    Resource & res;

    friend class SmallGraphicsObject;

    static const int chunkSize;
  };

#endif // TERRAIN_H
