#include "tnloptimize.h"

TnlOptimize::TnlOptimize()
{
}

void TnlOptimize::index( std::vector<MVertex>& v,
                         std::vector<uint16_t>& ibo ){
      std::unordered_map<MVertex, uint16_t, MVertex::hash > m;
      ibo.reserve( v.size() );
      ibo.resize( v.size() );

      for( size_t i=0; i<v.size(); ++i ){
        std::unordered_map<MVertex, uint16_t>::iterator it = m.find( v[i] );
        if( it==m.end() ){
          size_t sz = m.size();
          ibo[i]  = sz;
          m[v[i]] = sz;
          } else {
          ibo[i] = it->second;
          }
        }

      v.resize( m.size() );

      std::unordered_map<MVertex, uint16_t>::iterator
          i = m.begin(),
          e = m.end();

      for( ; i!=e; ++i )
        v[i->second] = i->first;
      }
