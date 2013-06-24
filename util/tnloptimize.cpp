#include "tnloptimize.h"

TnlOptimize::TnlOptimize()
{
}

void TnlOptimize::optimizeTerrain(std::vector<MVertex> &v){
  std::unordered_map<MVertex, uint16_t, MVertex::hash > m;

  for( size_t i=0; i<v.size(); ++i )
    ++m[v[i]];

  size_t wr = 0;
  for( size_t i=0; i<v.size(); ++i ){
    if( v[i].nz<0.9 || m[v[i]]!=6 ){
      v[wr] = v[i];
      ++wr;
      }
    }

  v.resize(wr);

  struct Cmp{
    static bool less( const MVertex& a,
                      const MVertex& b ){
      if( a.x < b.x )
        return 1;

      if( a.x > b.x )
        return 0;

      return a.y < b.y;
      }
    };

  std::sort( v.begin(), v.end(), Cmp::less );

  }
