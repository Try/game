#include "math.h"

#include <cstdint>

#include <limits>
#include <cmath>
#include <algorithm>

int Math::sqrt( int arg ) {
  int re = 46340; // ::sqrt( std::numeric_limits<int>::max() )
  int r = re/2;

  while( re*re!=arg && r>0 ){
    int q = re*re;
    if( q > arg )
      re -= r; else
      re += r;

    r/=2;
    }

  return re;
  }

int Math::distance(int x0, int y0, int x1, int y1) {
  x0 -= x1;
  y0 -= y1;
  return sqrt( x0*x0 + y0*y0 );
  }
