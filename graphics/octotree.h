#ifndef OCTOTREE_H
#define OCTOTREE_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <Tempest/Matrix4x4>

template< class T, class Cmp = std::less<T> >
class OcTree {
  public:
    class POcTree{
      OcTree * value;
      public:
        POcTree(OcTree * v = 0):value(v){}
        POcTree( const POcTree& ) = delete;
        ~POcTree(){ delete value; }

        void reset( OcTree * v = 0 ){
          delete value;
          value = v;
          }

        OcTree& operator * (){
          return *value;
          }

        const OcTree& operator * ()const {
          return *value;
          }

        operator bool () const { return value!=0; }

        OcTree* operator ->(){ return value; }
        const OcTree* operator ->() const{ return value; }

        POcTree& operator = ( const POcTree& ) = delete;
      };

    OcTree( int linearSize = leafLinearSize )
        :r(1.5*linearSize*1.45), linearSize(linearSize), owner(0){
        x = 0;
        y = 0;
        z = 0;

        count = 0;
        objects.reserve(64);
        }

    void reposition( const T &t, float x, float y, float z, float sZ ){
      OcTree& tn = node(t);
      OcTree&  n = node( x, y, z, sZ);

      if( &tn!=&n ){
        for( size_t i=0; i<tn.objects.size(); ++i ){
          size_t id = tn.objects.size()-i-1;

          if( tn.objects[id]==t ){
            tn.objects.erase( tn.objects.begin()+id );
            OcTree* p = &tn;
            while( p ){
              --p->count;
              p = p->owner;
              }

            //n.objects.push_back(t);
            n.objects.insert( std::upper_bound( n.objects.begin(),
                                                n.objects.end(),
                                                t, cmp ), t );
            enode(t);
            insNode(t, n);

            p = &n;
            while( p ){
              ++p->count;
              p = p->owner;
              }
            return;
            }
          }
        }
      }

    void insert( const T &t, float tx, float ty, float tz, float sz ){
      OcTree & n = node(tx, ty, tz, sz);

      n.objects.insert( std::upper_bound( n.objects.begin(),
                                          n.objects.end(),
                                          t, cmp ), t );
      //n.objects.push_back(t);
      insNode(t, n);

      OcTree* p = &n;

      while( p ){
        ++p->count;
        p = p->owner;
        }
      }

    bool remove( const T &t, float /*tx*/, float /*ty*/, float /*tz*/ ){
      OcTree& n = enode(t);

      auto i = std::remove( n.objects.begin(),
                            n.objects.end(), t );
      if( i!=n.objects.end() ){
        n.objects.resize( i-n.objects.begin() );
        OcTree* p = &n;
        while( p ){
          --p->count;
          p = p->owner;
          }

        return true;
        }

      return false;
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
        n->owner = this;

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

    size_t count;
  private:
    void insNode( const T & t, OcTree& n ){
      //nodes[t] = &n;

      //t->ustate.erase(this);
      t->ustate.insert(&n, this);
      }

    OcTree& node( const T & t ){
      //return *nodes[t];

      OcTree& n = *((OcTree*)t->ustate.at(this));
      return n;
      }

    OcTree& enode( const T & t ){
      //OcTree& n = *nodes[t];
      //nodes.erase(t);

      OcTree& n = *((OcTree*)t->ustate.erase(this));
      return n;
      }

    struct hash{
      template< class Tx >
      size_t operator ()( Tx t ) const {
        return size_t(t);
        }
      };

    std::unordered_map<T, OcTree*, hash > nodes;

    OcTree * owner;
    Cmp cmp;
  };

#endif // OCTOTREE_H
   
