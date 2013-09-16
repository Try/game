#include "operationdef.h"

#include "shadersource.h"

OperationDef::EqContext::EqContext() {
  alpha = 0;
  }

OperationDef::Node::Node() {
  std::fill( cvalue, cvalue+4, 0  );
  std::fill( swizle, swizle+4, '-');
  vertex = Tempest::Usage::Position;
  lang   = CompileOptions::Cg;
  }

OperationDef::OperationDef() {
  Node ftransform =
  node( Mul,
        node("mvpMatrix"),
        node(Add,
             node(0,0,0,1),
             node(Tempest::Usage::Position) ) );

  Node
  getScreenGl =
      node( ScreenTexture,
            node( Add,
                     node( Mul,
                       node(Div,
                         node(ftransform, "xy--"),
                         node(ftransform, "ww--") ),
                       node(0.5, 0.5) ),
                     node(0.5, 0.5) )
            );

  Node
  getScreenDx =
      node( ScreenTexture,
            node( Add,
                node( Add,
                         node( Mul,
                           node(Div,
                             node(ftransform, "xy--"),
                             node(ftransform, "ww--") ),
                           node(0.5, 0.5) ),
                         node(0.5, 0.5) ),
                  node("dxScreenOffset") )
            );

  getScreen = node( getScreenDx,
                    getScreenGl,
                    getScreenGl );

  addBlend = node( Add, getScreen, node(AnyNode) );

  auto alpha = node(Alpha),
       color = node(AnyNode);//node( node(Alpha), "xyz-");

  alphaBlend = node( Add,
                node( Mul, color, alpha ),
                node( Mul, getScreen, node( Sub, node(1), alpha ) ) );
  }

OperationDef::Node OperationDef::node( const std::string & str ) {
  Node n;
  n.type    = Uniform;
  n.uniform = str;

  return n;
  }

OperationDef::Node OperationDef::node( const Node& nn, const char s[4] ) {
  Node n;
  n.type      = Swizzle;
  n.swizle[0] = s[0];
  n.swizle[1] = s[1];
  n.swizle[2] = s[2];
  n.swizle[3] = s[3];

  n.node.push_back(nn);
  return n;
  }

OperationDef::Node OperationDef::node(float x, float y, float z, float w) {
  Node n;
  n.type      = Const;
  n.cvalue[0] = x;
  n.cvalue[1] = y;
  n.cvalue[2] = z;
  n.cvalue[3] = w;

  return n;
  }

OperationDef::Node OperationDef::node(Tempest::Usage::UsageType t) {
  Node n;
  n.type    = Vertex;
  n.vertex  = t;

  return n;
  }

OperationDef::Node OperationDef::node(OperationDef::NodeType t) {
  Node n;
  n.type = t;

  return n;
  }

OperationDef::Node OperationDef::node(NodeType t, const Node &a) {
  Node n;
  n.type = t;

  n.node.push_back(a);
  return n;
  }

OperationDef::Node OperationDef::node(NodeType t, const Node &a, const Node &b) {
  Node n;
  n.type = t;

  n.node.push_back(a);
  n.node.push_back(b);

  return n;
  }

OperationDef::Node OperationDef::node( const Node &a,
                                       const Node &b,
                                       const Node &c ) {
  Node n;
  n.type = Lang;

  n.node.push_back(a);
  n.node.push_back(b);
  n.node.push_back(c);

  return n;
  }

bool OperationDef::equal( const ShaderSource &s,
                          const OperationDef::Node &n,
                          const CompileOptions &opt) {
  EqContext c;
  return equal(s, n, opt, c);
  }

bool OperationDef::equal(const ShaderSource &s,
                          const OperationDef::Node &n,
                          const CompileOptions &opt,
                          EqContext &c ) {
  if( n.type==Lang ){
    return equal(s, n.node[ opt.lang ], opt);
    }

  if( n.type==AnyNode )
    return true;

  if( n.type==Alpha ){
    if( c.alpha )
      return *c.alpha == s;

    if( s.csize==1 ){
      c.alpha = &s;
      return 1;
      }

    return 0;
    }

  if( s.nodes.size()!=n.node.size() )
    return false;

  if( s.uniformName.size() && n.type==Uniform ){
    return s.uniformName==n.uniform;
    }

  if( s.type==ShaderSource::Operator && s.nodes.size()==2 ){
    if( s.opType==ShaderSource::Add && n.type==Add ){
      return equalBin(s,n, opt, c);
      }
    if( s.opType==ShaderSource::Sub && n.type==Sub ){
      return equalN(s,n, opt, c);
      }
    if( s.opType==ShaderSource::Div && n.type==Div ){
      return equalN(s,n, opt, c);
      }
    if( s.opType==ShaderSource::Mul && n.type==Mul ){
      return equalN(s,n, opt, c);
      }
    }

  if( s.type==ShaderSource::Constant && n.type==Const ){
    for( int i=0; i<s.csize; ++i )
      if( s.cvalue[i]!=n.cvalue[i] )
        return 0;

    for( int i=s.csize; i<4; ++i )
      if( n.cvalue[i]!=0 )
        return 0;

    return 1;
    }

  if( s.type==ShaderSource::Vertex && n.type==Vertex ){
    return s.vcomponent==n.vertex;
    }

  if( s.type==ShaderSource::Math && s.funcName.size()==0 && n.type==Swizzle ){
    for( int i=0; i<4; ++i )
      if( s.mathSW[i]!=n.swizle[i] )
        return 0;
    return equal(*s.nodes[0], n.node[0], opt, c);
    }

  if( n.type==ScreenTexture &&
      s.type==ShaderSource::TextureRect &&
      s.textureSemantic==ShaderSource::tsScreenData ){
    return equal(*s.nodes[0], n.node[0], opt, c);
    }

  return false;
  }

bool OperationDef::equalBin(const ShaderSource &s,
                             const OperationDef::Node &n,
                             const CompileOptions& opt , EqContext &c) {
  return (equal(*s.nodes[0], n.node[0], opt, c) && equal(*s.nodes[1], n.node[1], opt, c)) ||
         (equal(*s.nodes[1], n.node[0], opt, c) && equal(*s.nodes[0], n.node[1], opt, c));
  }

bool OperationDef::equalN(const ShaderSource &s,
                           const OperationDef::Node &n,
                           const CompileOptions& opt , EqContext &c) {
  return (equal(*s.nodes[0], n.node[0], opt, c) && equal(*s.nodes[1], n.node[1], opt, c));
  }



