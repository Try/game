#ifndef TNLOPTIMIZE_H
#define TNLOPTIMIZE_H

#include "model_mx.h"
#include <vector>
#include "stlconf.h"
#include <unordered_map>

class TnlOptimize {
  public:
    TnlOptimize();

    template< class T >
    static void index( std::vector<T>& v,
                       std::vector<uint16_t>& ibo ){
      std::unordered_map<T, uint16_t, MVertex::hash > m;
      ibo.reserve( v.size() );
      ibo.resize( v.size() );

      for( size_t i=0; i<ibo.size(); ++i ){
        typename std::unordered_map<T, uint16_t>::iterator it = m.find( v[i] );
        if( it==m.end() ){
          size_t sz = m.size();
          ibo[i]  = sz;
          m[v[i]] = sz;
          } else {
          ibo[i] = it->second;
          }
        }

      v.resize( m.size() );

      typename std::unordered_map<T, uint16_t>::iterator
          i = m.begin(),
          e = m.end();

      for( ; i!=e; ++i )
        v[i->second] = i->first;
      }

    static void optimizeTerrain( std::vector<MVertex>& v );
  };

#endif // TNLOPTIMIZE_H
