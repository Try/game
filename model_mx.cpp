#include "model_mx.h"

#include <cstring>
#include <cmath>
#include "util/ifstream.h"

#include "util/tnloptimize.h"

bool Model::hasFP16;

Tempest::VertexDeclaration::Declarator MVertexF::mkDecl() {
  Tempest::VertexDeclaration::Declarator d;

  d.add( Tempest::Decl::float4,  Tempest::Usage::Position )
   .add( Tempest::Decl::float4,  Tempest::Usage::Normal   )
   .add( Tempest::Decl::float2,  Tempest::Usage::BiNormal )
   .add( Tempest::Decl::float2,  Tempest::Usage::TexCoord )
   .add( Tempest::Decl::color,   Tempest::Usage::Color    );

  return d;
  }

const Tempest::VertexDeclaration::Declarator &MVertexF::decl() {
  static Tempest::VertexDeclaration::Declarator d = mkDecl();
  return d;
  }


Tempest::VertexDeclaration::Declarator MVertex::mkDecl() {
  Tempest::VertexDeclaration::Declarator d;

  d.add( Tempest::Decl::half4,  Tempest::Usage::Position )
   .add( Tempest::Decl::half4,  Tempest::Usage::Normal   )
   .add( Tempest::Decl::half2,  Tempest::Usage::BiNormal )
   .add( Tempest::Decl::float2, Tempest::Usage::TexCoord )
   .add( Tempest::Decl::color,  Tempest::Usage::Color    );

  return d;
  }

const Tempest::VertexDeclaration::Declarator &MVertex::decl() {
  static Tempest::VertexDeclaration::Declarator d = mkDecl();
  return d;
  }

size_t MVertexF::hash::operator ()(const MVertexF &mv) const {
  size_t ret = 0;
  char *v  = (char*)&mv;
  char *vx = v+sizeof(*this);

  for(; v!=vx; ++v)
    ret += *v;

  return ret;
  }

size_t MVertex::hash::operator ()(const MVertex &mv) const {
  size_t ret = 0;
  char *v  = (char*)&mv;
  char *vx = v+sizeof(*this);

  for(; v!=vx; ++v)
    ret += *v;

  return ret;
  }


void MVertexF::operator = ( const MVertex& f ){
  x = f.x;
  y = f.y;
  z = f.z;

  u = f.u;
  v = f.v;

  nx = f.nx;
  ny = f.ny;
  nz = f.nz;

  bx = f.bx;
  by = f.by;
  bz = f.bz;

  /*
  float bl = sqrt(bx*bx+by*by+bz*bz),
        nl = sqrt(nx*nx+ny*ny+nz*nz);

  nx /= nl;
  ny /= nl;
  nz /= nl;

  bx /= bl;
  by /= bl;
  bz /= bl;*/

  h = f.h;

  for( int i=0; i<4; ++i )
    color[i] = f.color[i];
  }


void MVertex::operator = ( const MVertexF& f ){
  x = f.x;
  y = f.y;
  z = f.z;

  u = f.u;
  v = f.v;

  nx = f.nx;
  ny = f.ny;
  nz = f.nz;

  bx = f.bx;
  by = f.by;
  bz = f.bz;

  h = f.h;

  for( int i=0; i<4; ++i )
    color[i] = f.color[i];
  }
