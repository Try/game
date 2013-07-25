#ifndef GUIASSEMBLY_H
#define GUIASSEMBLY_H

#include "vertexinputassembly.h"

class GuiAssembly : public VertexInputAssembly {
  public:
    GuiAssembly( const Tempest::VertexDeclaration::Declarator & decl,
                 ShaderSource::Lang lang );

    virtual std::string vComponent(int tagetSize , Tempest::Usage::UsageType vcomponent, int index, int &vsize);
    virtual int componentSize(Tempest::Usage::UsageType t);

    virtual bool contains(Tempest::Usage::UsageType t, int index);
  };

#endif // GUIASSEMBLY_H
