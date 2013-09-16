#ifndef OPERATIONDEF_H
#define OPERATIONDEF_H

#include <vector>
#include <string>

#include <Tempest/VertexDeclaration>
#include "compileoptions.h"

class ShaderSource;

class OperationDef {
  public:
    OperationDef();

    enum NodeType{
      AnyNode,
      Add,
      Sub,
      Mul,
      Div,
      Uniform,
      Vertex,
      Swizzle,
      ScreenTexture,
      Const,
      Lang,
      Alpha
      };

    struct Node{
      Node();

      NodeType type;
      std::string uniform;
      Tempest::Usage::UsageType vertex;
      CompileOptions::Lang lang;

      std::vector<Node> node;
      float cvalue[4];
      char  swizle[4];
      };

    Node getScreen, addBlend, alphaBlend;

    Node node( const std::string &str );
    Node node( const Node &n, const char s[4] );
    Node node( float x, float y = 0, float z = 0, float w = 0 );
    Node node( Tempest::Usage::UsageType t );
    Node node( NodeType t );
    Node node( NodeType t, const Node& a );
    Node node( NodeType t, const Node& a, const Node& b );
    Node node( const Node& a, const Node& b, const Node& c );

    struct EqContext{
      EqContext();

      const ShaderSource *alpha;
      };

    static bool equal(    const ShaderSource& s, const Node& n,
                          const CompileOptions& opt );

    static bool equal(    const ShaderSource& s, const Node& n,
                          const CompileOptions& opt,
                          EqContext& c );
    static bool equalBin( const ShaderSource& s, const Node& n,
                          const CompileOptions& opt,
                          EqContext& c );
    static bool equalN(   const ShaderSource& s, const Node& n,
                          const CompileOptions& opt,
                          EqContext& c );
  };

#endif // OPERATIONDEF_H
