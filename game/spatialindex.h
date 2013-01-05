#ifndef SPATIALINDEX_H
#define SPATIALINDEX_H

#include <vector>
#include <memory>

class GameObject;

class SpatialIndex {
  public:
    typedef std::shared_ptr<GameObject> PGameObject;
    SpatialIndex( int w, int h );

    void fill( std::vector<PGameObject> & obj );
    void clear();

    void solveColisions();

    template< class F, class ... Args >
    void visit( int x, int y, int R, F func, Args & ... args ) const {
      int lx = x - sizeMax - R,
          rx = x + sizeMax + R;

      int ly = y - sizeMax - R,
          ry = y + sizeMax + R;

      lx = std::max(0, lx);
      rx = std::min(w, rx);

      ly = std::max(0, ly);
      ry = std::min(h, ry);

      for( int y=ly; y<ry; ++y ){
        int l = psum[ lx+y*w ],
            r = psum[ rx+y*w ];

        for( int i=l; i<r; ++i ){
          if( obj[i] )
            func( *obj[i], args... );
          }
        }
      }

  private:
    std::vector<size_t>      psum;
    std::vector<GameObject*> obj;

    int w,h, sizeMax;

    void solveColisions(GameObject* , size_t id);

    static bool hasEffect(GameObject &tg, GameObject &obj );
    static void collision(GameObject &obj , GameObject &m, size_t id);
  };

#endif // SPATIALINDEX_H
