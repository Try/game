#ifndef OCTOTREE_H
#define OCTOTREE_H

#include "stlconf.h"
#include <memory>
#include <vector>
#include <unordered_map>

#include <Tempest/Matrix4x4>

template< class T >
class OcTree {
  public:
    typedef std::unique_ptr<OcTree> POcTree;
    OcTree( int linearSize = leafLinearSize )
        :r(1.5*linearSize*1.45), linearSize(linearSize){
        x = 0;
        y = 0;
        z = 0;
        }

    void reposition( const T &t, float x, float y, float z, float sZ ){
      OcTree& tn = *nodes[t];
      OcTree&  n = node( x, y, z, sZ);

      if( &tn!=&n ){
        for( size_t i=0; i<tn.objects.size(); ++i ){
          size_t id = tn.objects.size()-i-1;

          if( tn.objects[id]==t ){
            tn.objects[id] = tn.objects.back();
            tn.objects.pop_back();

            n.objects.push_back(t);
            nodes[t] = &n;
            return;
            }
          }
        }
      }

    void insert( const T &t, float tx, float ty, float tz, float sz ){
      OcTree & n = node(tx, ty, tz, sz);

      nodes[t] = &n;
      n.objects.push_back(t);
      }

    void remove( const T &t, float /*tx*/, float /*ty*/, float /*tz*/ ){
      OcTree& n = *nodes[t];
      nodes.erase(t);

      for( size_t i=0; i<n.objects.size(); ++i ){
        size_t id = n.objects.size()-i-1;

        if( n.objects[id]==t ){
          n.objects[id] = n.objects.back();
          n.objects.pop_back();
          return;
          }
        }
      }

    OcTree& node( float tx, float ty, float tz,
                  float sZ ){
      // return *this;

      if( linearSize<=leafLinearSize ){
        return *this;
        }

      tx -= x;
      ty -= y;
      tz -= z;

      int hsz = linearSize/2;
      int px = (tx < hsz)?0:1;
      int py = (ty < hsz)?0:1;
      int pz = (tz < hsz)?0:1;

      //px = std::max(0, std::min(px, 1) );
      //py = std::max(0, std::min(py, 1) );
      //pz = std::max(0, std::min(pz, 1) );

      POcTree& n = nested[px][py][pz];

      if( !n ){
        n.reset( new OcTree(linearSize/2) );

        n->x = x + px*n->linearSize;
        n->y = y + py*n->linearSize;
        n->z = z + pz*n->linearSize;
        }

      float dx = ( n->x + linearSize/4 - (x+tx)),
            dy = ( n->y + linearSize/4 - (y+ty)),
            dz = ( n->z + linearSize/4 - (z+tz));

      float inR = 1.5*linearSize/4;
      if( dx*dx+dy*dy+dz*dz > (inR - sZ)*(inR - sZ) ){
        return *this;
        }

      return n->node(tx+x, ty+y, tz+z, sZ);
      }

    std::vector<T> objects;
    POcTree nested[2][2][2];

    float   x, y, z;
    const float r;
    int   linearSize;

    static const int leafLinearSize = 1;

  private:
    struct hash{
      template< class Tx >
      size_t operator ()( Tx t ) const {
        return size_t(t);
        }
      };
    std::unordered_map<T, OcTree*, hash > nodes;
  };

#endif // OCTOTREE_H
   
