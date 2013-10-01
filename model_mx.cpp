#include "model_mx.h"

#include <cstring>
#include <cmath>
#include "util/ifstream.h"

#include "util/tnloptimize.h"

Tempest::VertexDeclaration::Declarator MVertexF::mkDecl() {
  Tempest::VertexDeclaration::Declarator d;

  d.add( Tempest::Decl::float4,  Tempest::Usage::Position )
   .add( Tempest::Decl::float4,  Tempest::Usage::Normal   )
   .add( Tempest::Decl::float4,  Tempest::Usage::BiNormal )
   .add( Tempest::Decl::float2, Tempest::Usage::TexCoord )
   .add( Tempest::Decl::color,  Tempest::Usage::Color    );

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
