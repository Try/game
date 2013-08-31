#include "buildinfunction.h"

#include <cmath>
#include <algorithm>

BuildInFunction::BuildInFunction(){
  add( "dot",       &BuildInFunction::dot,     asMinimaize );
  add( "step",      &BuildInFunction::step,    asMaximize  );
  add( "min",       &BuildInFunction::min,     asMinimaize );
  add( "max",       &BuildInFunction::max,     asMaximize  );
  add( "pow",       &BuildInFunction::pow,     asMaximize  );
  add( "reflect",   &BuildInFunction::reflect, asMaximize  );
  add( "cross",     &BuildInFunction::cross,   asSetTo3    );
  add( "step",      &BuildInFunction::step,    asMaximize  );

  add( "mix",        &BuildInFunction::mix,        asDontCare  );
  add( "smoothstep", &BuildInFunction::smoothstep, asMaximize  );

  add( "clamp",     &BuildInFunction::clamp,   asMaximize  );

  add( "saturate",  &BuildInFunction::saturate  );
  add( "length",    &BuildInFunction::length    );
  add( "normalize", &BuildInFunction::normalize );
  add( "sin",       &BuildInFunction::sin );
  add( "cos",       &BuildInFunction::cos );
  }

int BuildInFunction::outSz(const std::string &f, int s) {
  for( size_t i=0; i<f1.size(); ++i )
    if( f1[i]->name==f )
      return f1[i]->outSize(s);

  return 1;
  }

int BuildInFunction::outSz(const std::string &f, int s, int s1) {
  for( size_t i=0; i<f2.size(); ++i )
    if( f2[i]->name==f )
      return f2[i]->outSize(s,s1);

  return 1;
  }

int BuildInFunction::outSz(const std::string &f, int s, int s1, int s2) {
  for( size_t i=0; i<f2.size(); ++i )
    if( f3[i]->name==f )
      return f3[i]->outSize(s,s1,s2);

  return 1;
  }

BuildInFunction::ArgsSize BuildInFunction::argsSz(const std::string &f) {
  for( size_t i=0; i<f2.size(); ++i )
    if( f2[i]->name==f )
      return f2[i]->argsS;

  for( size_t i=0; i<f3.size(); ++i )
    if( f3[i]->name==f )
      return f3[i]->argsS;

  return asDontCare;
  }

void BuildInFunction::exec( const std::string & f,
                            const float *a, int sa,
                            float *out, int &osz ) const {
  for( size_t i=0; i<f1.size(); ++i )
    if( f1[i]->name==f ){
      f1[i]->exec(a, sa, out, osz);
      for( int i=osz; i<4; ++i )
        out[i] = 0;
      }
  }

void BuildInFunction::exec( const std::string & f,
                            const float *a, int sa,
                            const float *b, int sb,
                            float *out, int &osz ) const {
  for( size_t i=0; i<f2.size(); ++i )
    if( f2[i]->name==f ){
      f2[i]->exec(a, sa, b, sb, out, osz);
      for( int i=osz; i<4; ++i )
        out[i] = 0;
    }
  }

void BuildInFunction::exec( const std::string &f,
                            const float *a, int sa,
                            const float *b, int sb,
                            const float *c, int sc,
                            float *out, int &osz) const {
  for( size_t i=0; i<f3.size(); ++i )
    if( f3[i]->name==f ){
      f3[i]->exec(a, sa, b, sb, c, sc, out, osz);
      for( int i=osz; i<4; ++i )
        out[i] = 0;
    }
  }

int BuildInFunction::argsCount(const std::string &f) const {
  for( size_t i=0; i<f1.size(); ++i )
    if( f1[i]->name==f )
      return 1;

  for( size_t i=0; i<f2.size(); ++i )
    if( f2[i]->name==f )
    return 2;

  for( size_t i=0; i<f3.size(); ++i )
    if( f3[i]->name==f )
    return 3;

  return 0;
  }

void BuildInFunction::dot( const float *a, int sa,
                           const float *b, int sb,
                           float *out, int &osz) {
  int sz = std::min(sa, sb);
  osz    = 1;

  if( !out )
    return;

  out[0] = 0;
  for( int i=0; i<sz; ++i )
    out[0] += a[i]*b[i];
  }

void BuildInFunction::min( const float *a, int sa,
                           const float *b, int sb,
                           float *out, int &osz) {
  int sz = std::min(sa, sb);
  osz    = sz;

  if( !out )
    return;

  for( int i=0; i<sz; ++i )
    out[i] = std::min( a[i], b[i] );
  }

void BuildInFunction::max( const float *a, int sa,
                           const float *b, int sb,
                           float *out, int &osz) {
  int sz = std::max(sa, sb);
  osz    = sz;

  if( !out )
    return;

  for( int i=0; i<sz; ++i )
    out[i] = std::max( a[i], b[i] );
  }

void BuildInFunction::sin( const float *a, int sa,
                           float *out, int &osz) {
  osz = sa;

  if( !out )
    return;

  for( int i=0; i<sa; ++i )
    out[i] = std::sin(a[i]);
  }

void BuildInFunction::cos( const float *a, int sa,
                           float *out, int &osz) {
  osz = sa;

  if( !out )
    return;

  for( int i=0; i<sa; ++i )
    out[i] = std::cos(a[i]);
  }

void BuildInFunction::pow( const float *a, int sa,
                           const float *b, int sb,
                           float *out, int &osz) {
  int sz = std::max(sa, sb);
  osz    = sz;

  if( !out )
    return;

  for( int i=0; i<sz; ++i )
    out[i] = std::pow( a[i], b[i] );
  }

void BuildInFunction::reflect( const float *n, int sn,
                               const float *i, int si,
                               float *out, int &osz) {
  osz = std::max(sn,si);
  if( out==0 ){
    return;
    }

  float dt[4] = {}; int dsz = 0;
  dot(n, sn, i, si, dt, dsz);

  for( int r=0; r<osz; ++r )
    out[r] = i[r] - 2*n[r]*dt[0];
  }

void BuildInFunction::cross( const float *va, int sa,
                             const float *vb, int sb,
                             float *out, int &osz) {
  osz = 3;
  if( !out )
    return;

  float a[4] = {}, b[4] = {};
  for( int i=0; i<sa; ++i )
    a[i] = va[i];
  for( int i=0; i<sb; ++i )
    b[i] = vb[i];

  float a_yzx[] = {a[1], a[2], a[0] };
  float b_zxy[] = {b[2], b[0], b[1] };

  float a_zxy[] = {a[2], a[0], a[1] };
  float b_yzx[] = {b[1], b[2], b[0] };

  for( int i=0; i<osz; ++i )
    out[i] = a_yzx[i]*b_zxy[i] - a_zxy[i]*b_yzx[i];
  }

void BuildInFunction::mix( const float *va, int sa,
                           const float *vb, int sb,
                           const float *vc, int sc,
                           float *out, int &osz) {
  osz = std::max(sa, sb);
  if( sc!=1 )
    osz = std::min(osz, sc);

  if( !out )
    return;

  float a[4] = {}, b[4] = {}, c[4] = {};
  for( int i=0; i<sa; ++i )
    a[i] = va[i];
  for( int i=0; i<sb; ++i )
    b[i] = vb[i];
  for( int i=0; i<sc; ++i )
    c[i] = vc[i];

  if( sc==1 )
    std::fill( c, c+4, vc[0] );

  for( int i=0; i<osz; ++i )
    out[i] = a[i]*(1-c[i]) + b[i]*c[i];
  }

void BuildInFunction::clamp( const float *a, int sa,
                             const float *b, int sb,
                             const float *c, int sc,
                             float *out, int &osz) {
  osz = std::max(sa, std::max(sb,sc));
  if( !out )
    return;

  for( int i=0; i<osz; ++i )
    out[i] = std::max( b[i], std::min(c[i], a[i]) );
  }

void BuildInFunction::step( const float *a, int sa,
                            const float *b, int sb,
                            float *out, int &osz) {
  osz = std::max(sa, sb);
  if( !out )
    return;

  for( int i=0; i<osz; ++i )
    out[i] = b[i] >= a[i];
  }

void BuildInFunction::smoothstep( const float *a, int sa,
                                  const float *b, int sb,
                                  const float *x, int sx,
                                  float *out, int &osz) {
  osz = std::max(sa, std::max(sb, sx));
  if( !out )
    return;

  float t[4] = {};

  for( int i=0; i<osz; ++i )
    t[i] = (x[i]-a[i])/(b[i]-a[i]);

  saturate(t, osz, t, osz);

  for( int i=0; i<osz; ++i )
    out[i] = t[i]*t[i]*(3.0 - 2.0*t[i]);
  }

void BuildInFunction::saturate( const float *a, int sa,
                                float *out,     int &osz ) {

  osz = sa;

  if( !out )
    return;

  for( int i=0; i<osz; ++i )
    out[i] = std::max( 0.f, std::min(1.f, a[i]) );
  }

void BuildInFunction::length( const float *a, int sa,
                              float *out, int &osz ) {
  osz    = 1;

  if( !out )
    return;

  out[0] = 0;
  for( int i=0; i<sa; ++i )
    out[0] += a[i]*a[i];

  out[0] = sqrt( out[0] );
  }

void BuildInFunction::normalize( const float *a, int sa,
                                 float *out, int &osz) {
  osz    = sa;

  if( !out )
    return;

  double l = 0;
  for( int i=0; i<sa; ++i )
    l += a[i]*a[i];

  l = sqrt( l );

  for( int i=0; i<sa; ++i )
    out[i] = a[i]/l;
  }
