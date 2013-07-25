#include "guiassembly.h"

GuiAssembly::GuiAssembly( const Tempest::VertexDeclaration::Declarator &decl,
                          ShaderSource::Lang lang )
  :VertexInputAssembly(decl, lang) {

  }

std::string GuiAssembly::vComponent( int ts,
                                     Tempest::Usage::UsageType vcomponent,
                                     int index,
                                     int& vsize ) {
  if( vcomponent==Tempest::Usage::Position ){
    std::string s = VertexInputAssembly::vComponent(ts, vcomponent, index, vsize);
    vsize = 2;
    return s+".xy";
    }

  if( vcomponent==Tempest::Usage::TexCoord ){
    return VertexInputAssembly::vComponent( 2, Tempest::Usage::TexCoord,
                                            0, vsize);
    }

  if( vcomponent==Tempest::Usage::Color ){
    int d = 0;
    vsize = 4;
    return VertexInputAssembly::vComponent( 2, Tempest::Usage::TexCoord,
                                            1, d );
    }

  return VertexInputAssembly::vComponent(ts, vcomponent, index, vsize);
  }

int GuiAssembly::componentSize( Tempest::Usage::UsageType t ) {
  if( t==Tempest::Usage::Position )
    return 2;

  if( t==Tempest::Usage::TexCoord )
    return 2;

  if( t==Tempest::Usage::Color )
    return 4;

  return VertexInputAssembly::componentSize(t);
  }

bool GuiAssembly::contains(Tempest::Usage::UsageType t, int index) {
  if( t==Tempest::Usage::Position && index==0 )
    return 1;

  if( t==Tempest::Usage::TexCoord && (index==0||index==1) )
    return 1;

  if( t==Tempest::Usage::Color && index==0 )
    return 1;

  return 0;
  }
