#ifndef MXASSEMBLY_H
#define MXASSEMBLY_H

#include "vertexinputassembly.h"

class MxAssembly : public VertexInputAssembly {
  public:
    MxAssembly(const Tempest::VertexDeclaration::Declarator & decl , ShaderSource::Lang lang);

    virtual std::string vComponent(int tagetSize , Tempest::Usage::UsageType vcomponent, int index, int &vsize);
    virtual int componentSize(Tempest::Usage::UsageType t);
  };

#endif // MXASSEMBLY_H
