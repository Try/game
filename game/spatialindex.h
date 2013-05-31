#ifndef SPATIALINDEX_H
#define SPATIALINDEX_H

#include <vector>
#include <memory>
#include <cassert>

#include <Tempest/Utility>

class GameObject;

class SpatialIndex {
  public:
    typedef std::shared_ptr<GameObject> PGameObject;
    SpatialIndex( int w, int h );

    void fill( std::vector<PGameObject> & obj );
    void clear();

    void solveColisions( std::vector<PGameObject> &obj );

    template< class F, class ... Args >
    void visit( int x, int y, int R, F func, Args & ... args ) const {
      const int sz = TerrQSize;
      tree.visit(x/sz,y/sz, R, func, args...);
      /*
      int lx = x/qsize - (sizeMax + R)/detail - 1,
          rx = x/qsize + (sizeMax + R)/detail + 1;

      int ly = y/qsize - (sizeMax + R)/detail - 1,
          ry = y/qsize + (sizeMax + R)/detail + 1;

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
        }*/
      }

    void add( GameObject* );
    void del( GameObject* );
    void move( GameObject* obj,
               int x, int y,
               int nx, int ny );
  private:
    std::vector<size_t>      psum;
    std::vector<GameObject*> obj;

    struct ObjTree{
      ObjTree( int w, int h ):w(w), h(h), hw(w/2), hh(h/2){
        if( w<=nodeSZ || h<=nodeSZ )
          obj.reserve(16);
        count = 0;
        }

      void add( GameObject* ob, int x, int y ){
        node(x,y, 1).obj.push_back(ob);
        }

      void del( GameObject* ob, int x, int y ){
        std::vector<GameObject*>& obj = node(x,y, -1).obj;
        for( size_t i=0; i<obj.size(); ++i )
          if( obj[i]==ob ){
            obj[i] = obj.back();
            obj.pop_back();
            return;
            }
        assert(0);
        }

      void move( GameObject* obj,
                 int x, int y,
                 int nx, int ny ){
        if( &node(x,y)==&node(nx,ny) )
          return;

        del(obj,  x,  y);
        add(obj, nx, ny);
        }


      template< class F, class ... Args >
      void visit( int x, int y, int R, F f, Args & ... args ) const {
        if( count==0 )
          return;

        if( w<=nodeSZ || h<=nodeSZ ){
          for( size_t i=0; i<obj.size(); ++i )
            f( *obj[i], args... );
          }

        if( x-R <= hw ){
          if( y-R <= hh ){
            if( nested[0][0] )
              nested[0][0]->visit( x, y, R, f, args... );
            }
          if( y+R >= hh ) {
            if( nested[0][1] )
              nested[0][1]->visit( x, y-hh, R, f, args... );
            }
          }

        if( x+R >= hw ) {
          if( y-R <= hh ){
            if( nested[1][0] )
              nested[1][0]->visit( x-hw, y, R, f, args... );
            }
          if( y+R >= hh ){
            if( nested[1][1] )
              nested[1][1]->visit( x-hw, y-hh, R, f, args... );
            }
          }
        }

      ObjTree& node( int x, int y, int d = 0 ){
        count += d;
        if( w<=nodeSZ || h<=nodeSZ ){
          assert( (count-d)==int(obj.size()) );
          return *this;
          }

        int i = 0, r = 0,
            nw = hw, nh = hh,
            dx = 0, dy = 0;
        if( x>hw ){
          i = 1;
          nw = w-hw;
          dx = hw;
          }
        if( y>hh ){
          r = 1;
          hw = h-hh;
          dy = hh;
          }

        if( !nested[i][r] )
          nested[i][r].reset( new ObjTree(nw, nh) );

        return nested[i][r]->node( x-dx, y-dy, d );
        }

      int w,h, hw, hh;
      int count;
      std::unique_ptr<ObjTree> nested[2][2];
      std::vector<GameObject*> obj;

      static const int nodeSZ = 4;
      };

    ObjTree tree;

    int w,h, sizeMax;
    int rndVec;

    void solveColisions(GameObject* , size_t id);

    static bool hasEffect(GameObject &tg, GameObject &obj );
    static void collision(GameObject &obj , GameObject &m, int &id);

    static const int TerrQSize;
    static const int qsize;
    static const int detail;
  };

#endif // SPATIALINDEX_H
