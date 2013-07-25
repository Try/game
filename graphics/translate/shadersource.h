#ifndef SHADERSOURCE_H
#define SHADERSOURCE_H

#include <vector>
#include <string>
#include <memory>

#include <Tempest/VertexDeclaration>
#include "buildinfunction.h"

class VertexInputAssembly;

class ShaderSource {
  public:
    ShaderSource();

    enum Lang{
      Cg,
      GLSL
      };

    enum Type{
      Unknown,
      Constant,
      Operator,
      Texture,
      TextureRect,
      Vertex,
      Math,
      LibConnect,
      ExternLib,
      ShaderOutput
      };
    Type type;
    bool isVarying, isUniform, isTexture;

    char mathSW[4];

    int          hasTmp;
    mutable bool tmpUsed;

    float cvalue[4]; int csize, csize1;
    enum OperatorType{
      Add,
      Sub,
      Mul,
      Div
      } opType;

    enum ShaderOuputType{
      NonOut,
      Transform,
      Lighting
      } shOutType;

    enum TextureSemantic{
      tsDiffuse,
      tsNormal,
      tsSpecular,
      tsEmission,
      tsEnvi,
      tsAblimientOclusion,
      tsAlphaMask,
      tsCount
      };

    using VertexComponent = Tempest::Usage::UsageType;
    Tempest::Usage::UsageType vcomponent; int vcomponentIndex;
    int inputConnect;

    std::vector< std::shared_ptr<ShaderSource> > nodes;

    struct Code{
      std::string vs, fs;
      enum UniformType{
        ut,
        texture
        };

      struct Uniform{
        std::string  name;
        UniformType  type;
        std::wstring fileName;
        TextureSemantic texSemantic;
        };
      std::vector<Uniform> uniformVs, uniformFs;
      };
    Code code( Lang lang = Cg );
    Code code( const Tempest::VertexDeclaration::Declarator &d, Lang lang = Cg  );
    Code code( VertexInputAssembly& d, Lang lang = Cg  );

    std::string  name, uniformName, funcName;
    //const void*  texBlockId;
    std::wstring fileName;

    TextureSemantic textureSemantic;

    bool operator == ( const ShaderSource& s ) const;
    bool operator != ( const ShaderSource& s ) const;

    std::shared_ptr< std::wstring > wrkFolder;

    static const std::string& floatN(Lang l, int n);
    static const std::string& floatN(Lang l, int n, int m );
  private:
    int convStage;
    mutable bool usedInVs, usedInFs;

    struct Context{
      VertexInputAssembly* decl;

      std::vector<ShaderSource *> varying, uniform;
      std::vector<std::shared_ptr<ShaderSource>> nodes;

      Lang lang;
      };

    bool isFSCode() const;

    static const std::string & swizle(int sz);
    static std::string expand(const std::string &s, int oldS, int sz, const Context &cx);
    std::string src(const std::string &sep, const Context &v, bool vshader, int vecSz) const;
    std::string color(const std::string &sep, const Context &v) const;
    std::string transform(const std::string &sep, const Context &v) const;

    std::string varying(Context &v) const;
    std::string uniform(Context &v, bool vs) const;
    std::string compileTmp(Context& cx , const std::string &sep, bool vs) const;

    void preprocess(Context &cx);
    void fillNodes( Context &v );
    void optNodes( Context & v );
    //void mkCompact();
    bool mkVaryings( Context &cx,
                     bool forceDeepth );
    void mkTransformVarying( Context &cx );
    void mkUniforms( Context &cx,
                     bool forceDeepth );

    std::string compileOperator( const std::string &sep,
                                 const Context &v,
                                 bool vshader, int vecSz) const;
    std::string compileMath( const std::string &sep,
                             const Context &v,
                             bool vshader, int vecSz) const;

    std::string compileConst(const Context &v, int sz) const;
    std::string vComponent(const Context &cx, int sz) const;

    std::string varyingName(const ShaderSource *,
                             const Context &v , int sz) const;
    std::string varyingReg( const ShaderSource *,
                            const Context &v) const;

    std::string decl(VertexInputAssembly &d );

    int outSize() const;

    int componentSz(const Context &cx, Tempest::Usage::UsageType t ) const;
    bool isZero() const;

    static BuildInFunction bfunc;

    static const int vertexStage, fragmentStage, varyingStage;

    static const std::string& floatN(const Context &cx, int n );
    static const std::string& floatN(const Context &cx, int n, int m );

  friend class ObjectCode;
  };

#endif // SHADERSOURCE_H
