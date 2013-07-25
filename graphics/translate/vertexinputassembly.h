#ifndef VERTEXINPUTASSEMBLY_H
#define VERTEXINPUTASSEMBLY_H

#include <Tempest/VertexDeclaration>
#include <string>
#include "shadersource.h"

class VertexInputAssembly {
  public:
    VertexInputAssembly( const Tempest::VertexDeclaration::Declarator & decl,
                          ShaderSource::Lang lang );
    virtual ~VertexInputAssembly();

    virtual bool contains(Tempest::Usage::UsageType t , int index);
    virtual int componentSize( Tempest::Usage::UsageType t );

    virtual std::string vComponent(int tagetSize , Tempest::Usage::UsageType vcomponent, int index, int &vsize);
    virtual std::string vsInput();

    ShaderSource::Lang getLang() const;
  protected:
    const Tempest::VertexDeclaration::Declarator decl;

    struct DeclStr{
      std::string reg, name;
      };

    ShaderSource::Lang lang;
    const DeclStr& toString( Tempest::Usage::UsageType t ) const;
  };

#endif // VERTEXINPUTASSEMBLY_H
