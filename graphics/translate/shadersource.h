#ifndef SHADERSOURCE_H
#define SHADERSOURCE_H

#include <vector>
#include <memory>

#include <Tempest/VertexDeclaration>
#include <Tempest/RenderState>

#include "buildinfunction.h"
#include "compileoptions.h"

#include <memory>

class VertexInputAssembly;

class ShaderSource {
  public:
    ShaderSource();

    typedef CompileOptions::Lang Lang;

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
      ShaderOutput,
      BuildIn,
      Option
      };
    Type type;
    bool isVarying, isUniform, isTexture;

    char mathSW[4];

    mutable int  hasTmp;
    mutable bool isTmp;
    bool wasPreprocessed;

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
      tsScreenData,
      tsShadowMap,
      tsCount
      };

    typedef Tempest::Usage::UsageType VertexComponent;
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
        int slot;
        };
      std::vector<Uniform> uniformVs, uniformFs;

      Tempest::RenderState rs;
      };
    Code code(const CompileOptions &lang = CompileOptions() );
    Code code(const Tempest::VertexDeclaration::Declarator &d,
               const CompileOptions &lang = CompileOptions()  );
    Code code( VertexInputAssembly& d,
               const CompileOptions &lang = CompileOptions()  );

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

    std::bitset<Tempest::Usage::Count> vInputUsed;

    struct Context{
      VertexInputAssembly* decl;

      std::vector<ShaderSource *> varying, uniform;
      std::vector<std::shared_ptr<ShaderSource>> nodes;
      std::set<const ShaderSource*> tmp;

      Lang lang;
      const CompileOptions * opt;
      };

    static bool isGL( const Context& c );

    bool isFSCode() const;

    static const std::string & swizle(int sz);
    static std::string expand(const std::string &s, int oldS, int sz, const Context &cx);
    std::string src(const std::string &sep, Context &v, bool vshader, int vecSz) const;
    std::string color(const std::string &sep, Context &v) const;
    void preATest(Context &v) const;
    std::string atest(const std::string &sep, Context &v) const;
    std::string transform(const std::string &sep, Context &v) const;

    std::string varying(Context &v) const;
    std::string uniform(Context &v, bool vs) const;
    std::string compileTmp( const ShaderSource*,
                            Context& cx,
                            const std::string &sep, bool vs) const;
    std::string compileTmp(Context& cx, const std::string &sep, bool vs) const;

    void dependsList( std::set<const ShaderSource*>& d, bool vs ) const;

    void preprocess (Context &cx);
    void preprocessR(Context &cx, bool isVs);

    void fillNodes( Context &v );
    void optNodes( Context & v );
    void mkRState(Context & v , Code &c);
    //void mkCompact();
    bool mkVaryings( Context &cx,
                     bool forceDeepth );
    bool reduceVaryings(Context &cx , size_t count);
    void mkTransformVarying( Context &cx );
    void mkUniforms( Context &cx,
                     bool forceDeepth );

    std::string compileOperator(const std::string &sep,
                                 Context &v,
                                 bool vshader, int vecSz) const;
    std::string compileMath(const std::string &sep,
                             Context &v,
                             bool vshader, int vecSz) const;

    std::string compileConst(const Context &v, int sz) const;
    std::string vComponent(const Context &cx, int sz) const;

    std::string varyingName(const ShaderSource *,
                             const Context &v , int sz, bool nameOnly) const;
    std::string varyingReg( const ShaderSource *,
                            const Context &v) const;

    std::string decl(VertexInputAssembly &d );

    int outSize() const;
    bool isSwizzle() const;
    bool isMatrixOp() const;

    int componentSz(const Context &cx, Tempest::Usage::UsageType t ) const;
    bool isZero() const;
    bool isOne() const;

    void setToZero();
    void setToNode(int n);

    static BuildInFunction bfunc;
    static OperationDef    opDefs;

    static const int vertexStage, fragmentStage, varyingStage;

    static const std::string& floatN(const Context &cx, int n );
    static const std::string& floatN(const Context &cx, int n, int m );

    static std::string to_string( int i );
    static std::string to_string( float i );

    void argsSize( int &sz0,
                   int &sz1,
                   int &sz2 ) const;
    static void argsSize( int &sz0,
                          int &sz1,
                          int &sz2,
                          const std::string & f );

    void removeNull();

  friend class ObjectCode;
  };

#endif // SHADERSOURCE_H
