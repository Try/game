#include "vertexinputassembly.h"

#include <sstream>

VertexInputAssembly::
VertexInputAssembly(const Tempest::VertexDeclaration::Declarator &decl,
                    ShaderSource::Lang lang):decl(decl), lang(lang) {

  }

VertexInputAssembly::~VertexInputAssembly() {
  }

bool VertexInputAssembly::contains(Tempest::Usage::UsageType t, int index = 0) {
  for( int i=0; i<decl.size(); ++i )
    if( decl[i].usage==t && decl[i].index==index )
      return true;

  return false;
  }

int VertexInputAssembly::componentSize(Tempest::Usage::UsageType t) {
  static const int sz[] = {
    0, 1, 2, 3, 4,
    4,2,4,
    2,4
    };

  for( int i=0; i<decl.size(); ++i ){
    if( t==decl[i].usage ){
      return sz[int(decl[i].component)];
      }
    }

  return 0;
  }

std::string VertexInputAssembly::vComponent( int /*tagetSize*/,
                                             Tempest::Usage::UsageType vcomponent,
                                             int index,
                                             int& vsize ) {
  std::string v;
  if( lang==ShaderSource::Cg )
    v = "vs_in." + toString(vcomponent).name; else
    v = toString(vcomponent).name;

  vsize = componentSize(vcomponent);

  if( vcomponent==Tempest::Usage::TexCoord )
    v += std::to_string(index);
  /*
    for( int i=0; i<decl.size(); ++i )
      if( decl[i].usage==vcomponent )
        v += std::to_string( decl[i].index );*/

  return v;
  }


std::string VertexInputAssembly::vsInput() {
  static const int sz[] = {0,1,2,3,4, 4,2,4, 2,4};

  std::stringstream ss;
  if( lang==ShaderSource::Cg )
    ss << "struct VS_Input{\n";
  for( int i=0; i<decl.size(); ++i ){
    if( lang==ShaderSource::Cg )
      ss << "  ";
    if( lang==ShaderSource::GLSL )
      ss << "attribute ";

    Tempest::VertexDeclaration::Declarator::Element e = decl[i];
    ss << ShaderSource::floatN(lang, sz[e.component]) << " ";
    ss << toString(e.usage).name;//usage[e.usage].name;
    if( e.usage==Tempest::Usage::TexCoord )
      ss << std::to_string(e.index);

    if( lang==ShaderSource::Cg ){
      ss << " : ";
      ss << toString(e.usage).reg;//usage[e.usage].reg;
      if( e.usage==Tempest::Usage::TexCoord )
        ss << std::to_string(e.index);
      }

    ss << ";" << std::endl;
    }

  if( lang==ShaderSource::Cg )
    ss << "  };\n";

  return ss.str();
  }

ShaderSource::Lang VertexInputAssembly::getLang() const {
  return lang;
  }

const VertexInputAssembly::DeclStr&
  VertexInputAssembly::toString(Tempest::Usage::UsageType t) const {
  static const struct DeclStr usage[] = {
    {"POSITION",     "Position"},
    {"BlendWeight",  ""},   // 1
    {"BlendIndices", ""},  // 2
    {"NORMAL",       "Normal"},        // 3
    {"PSIZE",        ""},         // 4
    {"TEXCOORD",     "TexCoord"},      // 5
    {"Tangent",      ""},       // 6
    {"BINORMAL",     "BiNormal"},      // 7
    {"TessFactor",   ""},    // 8
    {"PositionT",    ""},     // 9
    {"COLOR",        "Color"},         // 10
    {"Fog",          ""},           // 11
    {"DEPTH",        "Depth"},         // 12
    {"Sample",       ""},        // 13
    {"Count",        ""}
    };

  return usage[t];
  }
