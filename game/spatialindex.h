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
      vstk.clear();
      tree.visit( x/sz, y/sz, R, vstk, func, args...);
      }

    void add( GameObject* );
    void del( GameObject* );
    void move( GameObject* obj,
               int x, int y,
               int nx, int ny );
  private:
    std::vector<size_t>      psum;
    std::vector<GameObject*> obj;

    struct ObjTree;

    struct VPoint{
      int x, y;
      const ObjTree * t;
      };

    mutable std::vector<VPoint> vstk;

    struct ObjTree{
      ObjTree( int w, int h ):w(w), h(h), hw(w/2), hh(h/2){
        if( w<=nodeSZ || h<=nodeSZ )
          obj.reserve(16);
        count = 0;

        nested[0][0] = 0;
        nested[0][1] = 0;
        nested[1][0] = 0;
        nested[1][1] = 0;
        }

      ~ObjTree(){
        delete nested[0][0];
        delete nested[0][1];
        delete nested[1][0];
        delete nested[1][1];
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
      void visit( int x, int y, int R,
                  std::vector<VPoint>& vstk,
                  F f, Args & ... args ) const {
        VPoint pt;
        pt.x = x;
        pt.y = y;
        pt.t = this;

        vstk.push_back(pt);

        while( vstk.size() ){
          const VPoint p = vstk.back();
          vstk.pop_back();

          const ObjTree * curr = p.t;
          for( size_t i=0; i<curr->obj.size(); ++i )
            f( *curr->obj[i], args... );

          int hw = curr->hw,
              hh = curr->hh;

          if( p.x-R <= hw ){
            if( p.y-R <= hh ){
              if( curr->nested[0][0] &&
                  curr->nested[0][0]->count ){
                pt.x = p.x;
                pt.y = p.y;
                pt.t = curr->nested[0][0];
                vstk.push_back(pt);
                //nested[0][0]->visit( x, y, R, f, args... );
                }
              }
            if( p.y+R >= hh ) {
              if( curr->nested[0][1] &&
                  curr->nested[0][1]->count ){
                pt.x = p.x;
                pt.y = p.y-hh;
                pt.t = curr->nested[0][1];
                vstk.push_back(pt);
                //nested[0][1]->visit( x, y-hh, R, f, args... );
                }
              }
            }

          if( p.x+R >= hw ) {
            if( p.y-R <= hh ){
              if( curr->nested[1][0] &&
                  curr->nested[1][0]->count  ){
                pt.x = p.x-hw;
                pt.y = p.y;
                pt.t = curr->nested[1][0];
                vstk.push_back(pt);
                //nested[1][0]->visit( x-hw, y, R, f, args... );
                }
              }
            if( p.y+R >= hh ){
              if( curr->nested[1][1] &&
                  curr->nested[1][1]->count ){
                pt.x = p.x-hw;
                pt.y = p.y-hh;
                pt.t = curr->nested[1][1];
                vstk.push_back(pt);
                //nested[1][1]->visit( x-hw, y-hh, R, f, args... );
                }
              }
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
          nested[i][r] = new ObjTree(nw, nh);

        return nested[i][r]->node( x-dx, y-dy, d );
        }

      int w,h, hw, hh;
      int count;
      ObjTree* nested[2][2];
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
