#include "mxassembly.h"

MxAssembly::MxAssembly( const Tempest::VertexDeclaration::Declarator &decl,
                        ShaderSource::Lang lang )
  :VertexInputAssembly(decl, lang) {

  }

std::string MxAssembly::vComponent( int ts,
                                    Tempest::Usage::UsageType vcomponent,
                                    int index,
                                    int& vsize ) {
  int d = 0;

  if( vcomponent==Tempest::Usage::Position ){
    std::string s = VertexInputAssembly::vComponent(ts, vcomponent, index, vsize);
    vsize = 3;
    return s+".xyz";
    }

  if( vcomponent==Tempest::Usage::TexCoord ){
    std::string s =
        ShaderSource::floatN(lang, 2) +
        "( " +
        VertexInputAssembly::vComponent(4, Tempest::Usage::Position, index, d) +
        ".w, " +
        VertexInputAssembly::vComponent(4, Tempest::Usage::Normal, index, d) +
        ".z" +
        " )";
    vsize = 2;
    return s;
    }

  if( vcomponent==Tempest::Usage::Normal ){
    std::string str =
        ShaderSource::floatN(lang, 3)+"("
        + VertexInputAssembly::vComponent(2, Tempest::Usage::Normal, index, d)
        + ".xy, "
        + VertexInputAssembly::vComponent(1, Tempest::Usage::TexCoord, index, d)
        + ".x )";
    vsize = 3;
    return str;
    }

  if( vcomponent==Tempest::Usage::BiNormal ){
    std::string s =
        ShaderSource::floatN(lang, 3)+"(" +
        VertexInputAssembly::vComponent(4, Tempest::Usage::Normal, index, d) +
        ".w, " +
        VertexInputAssembly::vComponent(1, Tempest::Usage::BiNormal, index, d) +
        ".x, " +
        VertexInputAssembly::vComponent(2, Tempest::Usage::TexCoord, index, d) +
        ".y" +
        " )";
    vsize = 3;
    return s;
    }

  return VertexInputAssembly::vComponent(ts, vcomponent, index, vsize);
  }

int MxAssembly::componentSize(Tempest::Usage::UsageType t) {
  if( t==Tempest::Usage::BiNormal ||
      t==Tempest::Usage::Normal   ||
      t==Tempest::Usage::Position )
    return 3;

  if( t==Tempest::Usage::TexCoord )
    return 2;

  return VertexInputAssembly::componentSize(t);
  }
