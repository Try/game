#include "shadersource.h"

#include <utility>
#include <sstream>
#include <map>
#include <unordered_set>
#include <cmath>

#include <Tempest/Model>
#include "objectcode.h"
#include "vertexinputassembly.h"

#include "operationdef.h"

BuildInFunction ShaderSource::bfunc;
OperationDef    ShaderSource::opDefs;

const int ShaderSource::fragmentStage = 2,
          ShaderSource::varyingStage  = 1,
          ShaderSource::vertexStage   = 0;

ShaderSource::ShaderSource() {
  type      = Unknown;
  shOutType = NonOut;
  opType    = Add;

  textureSemantic = tsDiffuse;

  csize  = 1;
  csize1 = 1;

  wasPreprocessed = false;

  isVarying = false;
  isUniform = false;
  isTexture = false;

  hasTmp    = false;
  tmpUsed   = false;

  std::fill( cvalue, cvalue+4, 0 );
  std::fill( mathSW, mathSW+4, '-');
  mathSW[0] = 'x';
  cvalue[3] = 1;

  vcomponent      = Tempest::Usage::TexCoord;
  vcomponentIndex = 0;
  //mathType   = mtX;
  //texBlockId = 0;
  convStage  = -1;

  inputConnect = -1;

  usedInVs = 0;
  usedInFs = 0;

  vInputUsed.reset();
  }

ShaderSource::Code ShaderSource::code( const CompileOptions& lang) {
  return code( Tempest::DefaultVertex::decl(), lang );
  }

ShaderSource::Code ShaderSource::code( const Tempest::VertexDeclaration::Declarator &d,
                                       const CompileOptions& opt ) {
  VertexInputAssembly v(d, opt.lang);
  return code( v, opt );
  }

ShaderSource::Code ShaderSource::code( VertexInputAssembly &d,
                                       const CompileOptions& opt ) {
  Context cx;
  cx.decl = &d;
  cx.lang = opt.lang;
  cx.opt  = &opt;

  preprocess (cx);
  preprocessR(cx, false);

  fillNodes(cx);

  {
    size_t id = 0;
    std::map< std::pair<TextureSemantic, size_t>, size_t> m;
    for( size_t i=0; i<cx.nodes.size(); ++i ){
      if( cx.nodes[i]->type==Texture ||
          cx.nodes[i]->type==TextureRect ){
        auto ix = std::make_pair( cx.nodes[i]->textureSemantic,
                                  cx.nodes[i]->vcomponentIndex);
        auto f = m.find( ix );

        const std::string& tt = ObjectCode::toString(cx.nodes[i]->textureSemantic);
        if( f==m.end() ){
          cx.nodes[i]->uniformName = "t"+tt+to_string(id);
          m[ ix ] = id;
          ++id;
          } else {
          cx.nodes[i]->uniformName = "t"+tt+to_string(f->second);
          }
        }
      }
    }

  Code c;
  optNodes(cx);
  mkRState(cx, c);

  mkVaryings(cx, true);
  reduceVaryings(cx, 9);

  mkUniforms(cx, false);
  mkTransformVarying(cx);

  {
  std::string scode = color("\n  ", cx),
              tcode = compileTmp(cx,"\n  ", false);

  std::stringstream fs;
  if( cx.lang==CompileOptions::GLSLES )
    fs << "precision mediump float;" << std::endl << std::endl;
  fs << varying(cx) << std::endl;

  if( cx.lang==CompileOptions::Cg ){
    fs << "struct FS_Output{" << std::endl
       << "  float4 color:COLOR;" << std::endl
       << "  };" << std::endl << std::endl;
    }

  if( cx.lang==CompileOptions::Cg ){
    fs << "float4 texture2D( sampler2D tex, float2 t ){" << std::endl
       << "  return tex2D(tex, float2(t.x, 1.0-t.y));" << std::endl
       << "  }" << std::endl << std::endl;

    fs << "float4 texture2D( sampler2DRect tex, float2 t ){" << std::endl
       << "  return texRECTlod(tex, float4(t.x, 1.0-t.y, 0.0, 0.0));" << std::endl
       << "  }" << std::endl << std::endl;

    fs << "FS_Output main( VS_Output vs";
    fs << uniform(cx, false);
    } else {
    fs << uniform(cx, false) << std::endl;
    fs << "void main(){";
    }

  if( cx.lang==CompileOptions::Cg ){
    fs << " ) {" << std::endl;
    fs << "  FS_Output fs;" << std::endl;
    } else {
    fs << std::endl;
    }

  fs << "  " << tcode << scode;

  if( cx.lang==CompileOptions::Cg )
    fs << "return fs;\n  }" << std::endl; else
    fs << "}";

  c.fs = fs.str();
  }

  {
    std::stringstream ss;
    for( size_t i=0; i<cx.varying.size(); ++i ){
      if( cx.lang==CompileOptions::Cg )
        ss << "vs.";

      int sz = cx.varying[i]->csize;
      if( cx.varying[i]->shOutType==Transform )
        sz = 4;

      ss << varyingName(cx.varying[i],cx, sz, true)
         << " = "
         << cx.varying[i]->src("\n  ", cx, true, sz) <<";\n\n  ";
      }

    std::stringstream vs;
    vs << decl(*cx.decl) << std::endl
       << varying(cx)    << std::endl;

    std::string tCode = compileTmp(cx,"\n  ", true);

    if( cx.lang==CompileOptions::Cg ){
      vs << "VS_Output main( VS_Input vs_in";
      vs << uniform(cx, true) << " ) {" << std::endl;
      vs << "  VS_Output vs;" << std::endl << "  ";
      } else {
      vs << uniform(cx, true) << std::endl << std::endl;
      vs << "void main() {" << std::endl << "  ";
      }

    vs << tCode;
    vs << ss.str();
    if( cx.lang==CompileOptions::Cg )
      vs << "return vs;" << std::endl <<"  ";
    vs << "}" << std::endl;

    c.vs = vs.str();
    }

  for( size_t i=0; i<cx.uniform.size(); ++i ){
    ShaderSource* s = cx.uniform[i];

    Code::Uniform u;
    u.name     = s->uniformName;
    u.fileName = s->fileName;
    u.slot     = s->vcomponentIndex;

    if( s->type==Texture || s->type==TextureRect )
      u.type = Code::texture; else
      u.type = Code::ut;

    u.texSemantic = s->textureSemantic;

    if( s->usedInFs ){
      bool opt = false;
      for( size_t r=0; r<i; ++r )
        if( cx.uniform[r]->uniformName==s->uniformName &&
            cx.uniform[r]->usedInFs )
          opt = true;
      if( !opt )
        c.uniformFs.push_back(u);
      }
    if( s->usedInVs ){
      bool opt = false;
      for( size_t r=0; r<i; ++r )
        if( cx.uniform[r]->uniformName==s->uniformName &&
            cx.uniform[r]->usedInVs )
          opt = true;
      if( !opt )
        c.uniformVs.push_back(u);
      }
    }

  return c;
  }

std::string ShaderSource::compileTmp( const ShaderSource* tmp,
                                      ShaderSource::Context &cx,
                                      const std::string &sep,
                                      bool vs ) const {
  int tNum = tmp->hasTmp;
  tmp->hasTmp  = 0;
  std::string s = tmp->src(sep, cx, vs, tmp->csize);

  //std::string depends = compileTmp(cx, sep, vs);
  tmp->hasTmp = tNum;
  return s;
  }

std::string ShaderSource::compileTmp( ShaderSource::Context &cx,
                                      const std::string &sep,
                                      bool vs ) const {
  std::vector< std::set<const ShaderSource*> > lv;
  lv.reserve(8);
  lv.resize(1);
  std::swap( cx.tmp, lv[0] );

  std::set<const ShaderSource*> all;
  while( lv.back().size() ){
    lv.resize(lv.size()+1);
    const std::set<const ShaderSource*> &t = lv[lv.size()-2];

    for( auto i = t.begin(); i!=t.end(); ++i ){
      (*i)->dependsList(lv.back(), vs);
      }
    }

  std::stringstream ss;
  all.clear();

  std::set<int> compiled;
  for( size_t i=0; i<lv.size(); ++i ){
    const std::set<const ShaderSource*> &t = lv[lv.size()-i-1];

    for( auto i = t.begin(); i!=t.end(); ++i )
      if( compiled.find( (*i)->hasTmp)==compiled.end() ){
        ss << floatN(cx, (*i)->csize);

        ss << " "
           << "tmp"
           << (*i)->hasTmp
           << " = ";

        ss << compileTmp((*i), cx, sep, vs);
        ss << ";\n" << sep;

        compiled.insert((*i)->hasTmp);
        }
    }

  cx.tmp.clear();
  return ss.str();
  }

void ShaderSource::dependsList(std::set<const ShaderSource *> &d, bool vs) const {
  for( size_t i=0; i<nodes.size(); ++i ){
    const ShaderSource* ss = nodes[i].get();

    bool stage = (ss->convStage==varyingStage || ss->convStage==vertexStage);
    if( !( !vs && stage ) ){
      if( ss->hasTmp )
        d.insert(ss);
      ss->dependsList(d,vs);
      }
    }
  }

bool ShaderSource::operator ==(const ShaderSource &s) const {
  if( this==&s )
    return 1;

  if( nodes.size()!=s.nodes.size() )
    return 0;

  for( size_t i=0; i<nodes.size(); ++i )
    if( *nodes[i]!=*s.nodes[i] )
      return 0;

  int cs0 = convStage,
      cs1 = s.convStage;
/*
  if( cs0==varyingStage ){
    cs0 = cs1;
    }

  if( cs1==varyingStage ){
    cs1 = cs0;
    }*/

  return std::tie( s.type, s.isVarying, s.isUniform,
                   s.cvalue[0], s.cvalue[1], s.cvalue[2], s.cvalue[3],
                   s.csize, s.csize1,
                   s.opType, s.vcomponent,
                   //s.name,
                   s.uniformName,
                   s.textureSemantic, s.funcName,
                   cs1,
                   //s.shOutType,
                   s.mathSW[0], s.mathSW[1], s.mathSW[2], s.mathSW[3] ) ==
        std::tie( type, isVarying, isUniform,
                  cvalue[0], cvalue[1], cvalue[2], cvalue[3],
                  csize, csize1,
                  opType, vcomponent,
                  //name,
                  uniformName,
                  textureSemantic, funcName,
                  cs0,
                  //shOutType,
                  mathSW[0], mathSW[1], mathSW[2], mathSW[3] );
  }

bool ShaderSource::operator !=(const ShaderSource &s) const {
  return !(*this==s);
  }

const std::string &ShaderSource::swizle(int sz) {
  static const std::string s[4] = {
    ".x",
    ".xy",
    ".xyz",
    ".xyzw"
    };

  return s[sz-1];
  }

std::string ShaderSource::expand( const std::string &s,
                                  int oldS, int sz,
                                  const Context & cx ) {
  if( oldS>sz ){
    if( oldS==1 ){
      std::stringstream ss;
      ss << floatN(cx, sz) <<"(";
      for( int i=1; i<sz; ++i ){
        ss << s;
        if( i+1==sz )
          ss <<")"; else
          ss <<", ";
        }

      return ss.str();
      }
    return s + swizle(sz);
    }

  if( oldS==sz )
    return s;

  if( sz==1 )
    return s;

  std::stringstream ss;

  if( cx.lang==CompileOptions::Cg )
    ss << "float"; else
    ss << "vec";

  ss << sz <<"( " << s;
  for( int i=oldS; i<sz; ++i )
    ss << ", 0.0";
  ss << " )";

  return ss.str();
  }

bool ShaderSource::isFSCode() const{
  return !isTexture && isVarying;
  }

std::string ShaderSource::src( const std::string & sep,
                               Context &cx,
                               bool vshader,
                               int vecSz ) const {
  std::string str;
  if( vshader )
    usedInVs = true; else
    usedInFs = true;

  bool ccVarying = !vshader && (convStage==varyingStage || convStage==vertexStage);

  if( type==BuildIn ){
    return name;
    } else
  if( hasTmp && !ccVarying ){
    tmpUsed = true;
    cx.tmp.insert(this);
    return expand( "tmp" + to_string(hasTmp), csize, vecSz, cx );
    } else
  if( type==Texture || type==TextureRect ){
    if( nodes.size()==0 )
      return expand("0.0", 1, vecSz, cx);

    str = "texture2D(";
    str += uniformName +", "+nodes[0]->src(sep, cx, vshader, 2) +" )";
    /*
    if( type==TextureRect && cx.lang==CompileOptions::Cg ){
      str = "texRECTlod( ";
      str += uniformName +", invTexCoord("+nodes[0]->src(sep, cx, vshader, 4) +") )";
      } else {
      if( cx.lang==CompileOptions::Cg ){
        str = "tex2D(";
        str += uniformName +", invTexCoord("+nodes[0]->src(sep, cx, vshader, 2) +") )";
        } else {
        str = "texture2D(";
        str += uniformName +", "+nodes[0]->src(sep, cx, vshader, 2) +" )";
        }
      }*/

    if( vecSz!=4 )
      str += swizle(vecSz);
    return str;
    } else
  if( ccVarying ){
    if( cx.lang==CompileOptions::Cg )
      return varyingName(this, cx, vecSz, false); else
      return varyingName(this, cx, vecSz, false);
    } else
  if( !isTexture && isUniform && !isVarying && nodes.size()==0){
    if( nodes.size() ){
      //return "U";
      }
    return expand( uniformName, csize, vecSz, cx );
    } else
  if( type==Operator ){
    return compileOperator(sep, cx, vshader, vecSz);
    } else
  if( type==Math ){
    return compileMath(sep,cx, vshader, vecSz);
    } else
  if( type==Constant || type==Unknown ) {
    return compileConst(cx,vecSz);
    } else
  if( type==ShaderOutput ){
    return compileConst(cx,vecSz);
    } else
  if( type==Vertex){
    return vComponent(cx, vecSz);
    } else {

    for( size_t i=0; i<nodes.size(); ++i )
      str += nodes[i]->src(sep, cx, vshader, vecSz)+";" + sep;
    }

  return str;
  }

std::string ShaderSource::color( const std::string &sep,
                                 Context &v ) const {
  static const std::string fr[CompileOptions::LangCount] = {
    "fs.color = ",
    "gl_FragColor = ",
    "gl_FragColor = "
    };

  for( size_t i=0; i<nodes.size(); ++i )
    if( nodes[i]->shOutType==Lighting ){
      ShaderSource &cl = *nodes[i];
      return fr[v.lang] + cl.src(sep, v, false, 4) +";"+ sep;
      }

  if( v.lang==CompileOptions::Cg )
    return fr[0]+"float4(0.0);"+sep; else
    return fr[1]+"vec4(0.0);"+sep;
  }

std::string ShaderSource::transform( const std::string &sep,
                                     Context &v) const {
  for( size_t i=0; i<nodes.size(); ++i )
    if( nodes[i]->shOutType==Transform ){
      return "vs.position = " + nodes[i]->src(sep, v, true, 4) +";"+ sep;
      }

  return "vs.position = float4(0.0);"+sep;
  }

std::string ShaderSource::compileConst( const Context &v, int iSz ) const {
  std::stringstream ss;
  ss.setf(std::ios_base::showpoint );

  int sz = iSz;
  ss << floatN(v, sz);

  ss <<"( ";
  for( int i=0; i<sz; ++i ){
    if( i )
      ss <<", ";

#ifdef isnan
    if( isinf(cvalue[i]) || 
        isnan(cvalue[i]) )
#else
    if( std::isinf(cvalue[i]) || 
      std::isnan(cvalue[i]) )
#endif
      ss << "0.0/*nan*/"; else
      ss << cvalue[i];
    }

  ss <<" )";

  return ss.str();
  }

std::string ShaderSource::vComponent( const Context &cx, int sz ) const {
  int vsz = 0;
  std::string r = cx.decl->vComponent(sz, vcomponent, vcomponentIndex, vsz);
  return expand(r, vsz, sz, cx);
  }

std::string ShaderSource::varyingReg( const ShaderSource *s,
                                      const Context &cx) const {
  const std::vector<ShaderSource*> &v = cx.varying;
  int id=0, n = 0;
  for( size_t i=0; i<v.size(); ++i ){
    if( v[i]==s ){
      id = n;
      }

    if( v[i]->shOutType==Transform ){
      if( v[i]==s )
        return "POSITION";
      } else {
      ++n;
      }
    }

  return "TEXCOORD" + to_string(id);
  }

std::string ShaderSource::decl( VertexInputAssembly &d ) {
  return d.vsInput();
  }

std::string ShaderSource::varyingName( const ShaderSource *s,
                                       const Context &cx,
                                       int sz,
                                       bool nameOnly ) const {
  const std::vector<ShaderSource*>& v = cx.varying;
  int id=0, n = 0;
  for( size_t i=0; i<v.size(); ++i ){
    if( v[i]==s ){
      id = n;
      }

    if( v[i]->shOutType==Transform ){
      if( v[i]==s ){
        if( cx.lang==CompileOptions::Cg )
          return "position"; else
          return "gl_Position";
        }
      } else {
      ++n;
      }
    }

  if( nameOnly )
    return "v" + to_string(id);

  std::string name;
  if( cx.lang==CompileOptions::Cg )
    name = "vs.v" + to_string(id); else
    name = "v" + to_string(id);
  return expand( name, s->csize, sz, cx );
  }

std::string ShaderSource::varying( Context &v ) const {
  std::stringstream ss;

  if( v.lang==CompileOptions::Cg )
    ss << "struct VS_Output{\n";

  for( size_t i=0; i<v.varying.size(); ++i )
    if( v.lang==CompileOptions::Cg || v.varying[i]->shOutType!=Transform ){
      if( v.lang==CompileOptions::Cg )
        ss << "  ";
      if( isGL(v) )
        ss << "varying ";

      if( v.varying[i]->shOutType!=Transform )
        ss << floatN(v, v.varying[i]->csize) <<" "; else
        ss << floatN(v, 4) <<" ";

      if( !isGL(v) && v.varying[i]->csize==1 )
        ss <<" ";

      if( isGL(v) && v.varying[i]->csize>1 )
        ss <<" ";

      ss << varyingName(v.varying[i], v, v.varying[i]->csize, true);

      if( v.lang==CompileOptions::Cg )
        ss << " : " << varyingReg (v.varying[i], v);
      ss << ";\n";
      }

  if( v.lang==CompileOptions::Cg )
    ss << "  };\n";

  return ss.str();
  }

std::string ShaderSource::uniform( Context &v,
                                   bool vs ) const {
  std::stringstream ss;
  std::unordered_set< std::string > used;

  for( size_t i=0; i<v.uniform.size(); ++i ){
    ShaderSource *s = v.uniform[i];
    if( ((vs && s->usedInVs) || (!vs && s->usedInFs)) &&
         (used.find(s->uniformName)==used.end() )) {
      used.insert( s->uniformName );

      if( v.lang==CompileOptions::Cg )
        ss << ",\n    uniform "; else
        ss << "uniform";

      if( s->isTexture ){
        if( v.lang==CompileOptions::GLSLES)
          ss << "     ";
        if( s->type==TextureRect && v.lang==CompileOptions::Cg )
          ss << " sampler2DRect "; else
          ss << " sampler2D ";
        } else {
        if( v.lang==CompileOptions::GLSLES)
          ss << " lowp";
        ss << " " << floatN(v, s->csize, s->csize1) <<" ";
        }

      ss << v.uniform[i]->uniformName;

      if( isGL(v) )
        ss <<";"<< std::endl;
      }
    }

  return ss.str();
  }

void ShaderSource::removeNull(){
  int sz = 0;
  for( size_t i=0; i<nodes.size(); ++i )
    if( nodes[i]!=0 ){
      nodes[sz] = nodes[i];
      ++sz;
      }
  nodes.resize(sz);
  }

void ShaderSource::preprocess( Context & cx ) {
  if( wasPreprocessed )
    return;
  wasPreprocessed = true;

  while( type==ExternLib ){
    for( size_t i=1; i+1<nodes.size(); ++i )
      nodes[i] = nodes[i+1];
    if( nodes.size()>=2 )
      nodes.pop_back();

    ObjectCode c;
    c.wrkFolder = wrkFolder;
    ShaderOuputType t = shOutType;
    //c.loadFromFile( fileName.data() );
    std::wstring file = *c.wrkFolder+L"/lib/"+fileName;
    c.loadFromFile( file.data() );
    *this = *c.codeOf(0, nodes);
    shOutType = t;
    //removeNull();
    }

  removeNull();

  if( type==Texture || type==TextureRect ){
    csize = 4;
    }

  if( type==Texture || type==TextureRect ){
    std::shared_ptr<ShaderSource> s = std::make_shared<ShaderSource>();
    s->type  = Math;
    s->csize = 2;

    const char* m = "xy--";
    std::copy( m, m+4, s->mathSW );
    std::swap( s->nodes, nodes );
    nodes.push_back(s);
    }

  if( (type==Math     && !isSwizzle()  )||
      (type==Operator && !isMatrixOp() && nodes.size()==2 ) ){
    for( size_t i=0; i<nodes.size(); ++i )
      nodes[i]->preprocess(cx);

    int sz[3] = {};
    if( type==Operator ){
      if( opType==Add || opType==Sub ){
        sz[0] = std::max( nodes[0]->csize, nodes[1]->csize );
        } else
      if( opType==Mul ){
        sz[0] = std::min( nodes[0]->csize, nodes[1]->csize );
        } else
      if( opType==Div ){
        sz[0] = nodes[0]->csize;
        }

      sz[1] = sz[0];
      csize = sz[0];
      } else {
      argsSize( sz[0], sz[1], sz[2] );
      }

    for( size_t i=0; i<nodes.size(); ++i ){
      if( nodes[i]->csize!=sz[i] && nodes[i]->csize==1 ){
        std::shared_ptr<ShaderSource> s = std::make_shared<ShaderSource>();
        s->type  = Math;
        s->csize = sz[i];

        const char* m = "xyzw";
        std::copy( m, m+4, s->mathSW );
        std::fill( s->mathSW+sz[i], s->mathSW+4, '-' );

        //std::swap( s->nodes, nodes[i]->nodes );
        s->nodes.push_back( nodes[i] );
        nodes[i] = s;
        }
      }
    }

  while( type==Vertex && nodes.size()>0 ){
    ShaderOuputType t = shOutType;
    ShaderSource tmp = *nodes[0];
    *this = tmp;
    shOutType = t;
    }

  if( type==Vertex ){
    vInputUsed.set(vcomponent, 1);
    }

  while( type==Option ){
    bool cnd = ( cx.opt->options.find(name) != cx.opt->options.end() );
    size_t id = cnd?1:0;

    if( id<nodes.size() ){
      ShaderOuputType t = shOutType;
      nodes[id]->preprocess(cx);
      ShaderSource tmp = *nodes[id];
      *this = tmp;
      shOutType = t;
      } else {
      nodes.clear();
      type = Constant;
      std::fill(cvalue, cvalue+4, 0);
      csize = 1;
      }
    }

  bool c = true;
  if( shOutType!=NonOut ){
    isVarying  = true;
    }

  for( size_t i=0; i<nodes.size(); ++i ){
    nodes[i]->preprocess(cx);
    c &= ( nodes[i]->type==Constant );

    isVarying  |= nodes[i]->isVarying;
    isUniform  |= nodes[i]->isUniform;
    isTexture  |= nodes[i]->isTexture;
    vInputUsed |= nodes[i]->vInputUsed;

    convStage = std::max(convStage, nodes[i]->convStage);
    }

  if( shOutType>Transform ){
    convStage = std::max(varyingStage, convStage);
    }

  if( isTexture ){
    convStage = fragmentStage;
    if( nodes.size() && nodes[0]->isVarying )
      nodes[0]->convStage = std::max(varyingStage, nodes[0]->convStage);
    }

  if( isVarying )
    convStage = std::max( vertexStage, convStage );

  if( uniformName=="dxScreenOffset" && cx.lang!=CompileOptions::Cg ){
    setToZero();
    }

  if( type==Operator && nodes.size()!=2 ){
    type  = Constant;

    if( nodes.size()==1 ){
      setToNode(0);
      } else {
      csize = 1;
      std::fill(cvalue, cvalue+4, 0.f);
      nodes.clear();
      }
    }

  if( type==Operator ){
    if( opType==Add || opType==Sub ){
      for( size_t i=0; i<nodes.size(); ++i ){
        csize  = std::max( csize,  nodes[i]->csize );
        csize1 = std::max( csize1, nodes[i]->csize1 );
        }

      if( nodes[1]->isZero() ){
        if( nodes[0]->isZero() )
          setToZero(); else
          setToNode(0);
        }
      }
    if( opType==Mul ){
      if( nodes[0]->isZero() || nodes[1]->isZero() ){
        setToZero();
        } else
        if( nodes[1]->isOne() && (nodes[0]->csize <= nodes[1]->csize) ){
        setToNode(0);
        } else
      if( nodes[0]->isOne() && (nodes[1]->csize <= nodes[0]->csize) ){
        setToNode(1);
        } else {
        csize  = nodes[0]->csize;
        csize1 = nodes[0]->csize1;

        for( size_t i=0; i<nodes.size(); ++i ){
          csize  = std::min( csize,  nodes[i]->csize );
          csize1 = std::min( csize1, nodes[i]->csize1 );
          }

        if( nodes[0]->csize1>1 ){
          csize = nodes[0]->csize1;
          }
        if( nodes[1]->csize1>1 ){
          csize = nodes[1]->csize;
          }
        }
      }
    if( opType==Div ){
      if( nodes[0]->isZero() ){
        setToZero();
        } else
      if( nodes[1]->isOne() && (nodes[0]->csize <= nodes[1]->csize) ){
        setToNode(0);
        } else {
        for( size_t i=0; i<nodes.size(); ++i ){
          csize  = nodes[0]->csize;
          csize1 = nodes[0]->csize1;
          }
        }
      }
    }

  if( type==Math && nodes.size()==0 ){
    type = Constant;
    std::fill(cvalue, cvalue+4, 0.f);
    }

  if( type==Math &&
      funcName.size() &&
      bfunc.argsCount(funcName)!=int(nodes.size()) ){
    type = Constant;
    std::fill(cvalue, cvalue+4, 0.f);
    nodes.clear();
    }

  if( type==Math ){
    if( bfunc.argsCount(funcName)==0 ){
      for( int i=0; i<4; ++i )
        if( mathSW[i]!='-' )
          csize = i+1;

      const char *vv = "xyzw";
      bool exp = true;
      for( int i=0; i<csize; ++i )
        exp &= ( mathSW[i]==vv[i] );

      bool swswNode = nodes[0]->type==Math && nodes[0]->funcName.size()==0;
      if( swswNode ){
        for( int i=0; i<csize; ++i )
          swswNode &= (nodes[0]->mathSW[i]==mathSW[i]);
        }

      if( (exp && csize>=nodes[0]->csize) ||
          ( swswNode ) ){
        ShaderOuputType t = shOutType;
        ShaderSource tmp = *nodes[0];
        *this = tmp;
        shOutType = t;
        }
      } else
    if( bfunc.argsCount(funcName)==1 ){
      if( nodes[0]->type==Constant ){
        bfunc.exec( funcName,
                    nodes[0]->cvalue, nodes[0]->csize,
                    cvalue, csize );
        type = Constant;
        nodes.clear();
        } else {
        csize = bfunc.outSz( funcName, nodes[0]->csize );
        }
      } else
    if( bfunc.argsCount(funcName)==2 ){
      if( nodes[0]->type==Constant && nodes[1]->type==Constant ){
        bfunc.exec( funcName,
                    nodes[0]->cvalue, nodes[0]->csize,
                    nodes[1]->cvalue, nodes[1]->csize,
                    cvalue, csize );
        type = Constant;
        nodes.clear();
        } else {
        csize = bfunc.outSz( funcName,
                             nodes[0]->csize,
                             nodes[1]->csize );
        }
      }else
    if( bfunc.argsCount(funcName)==3 ){
      if( nodes[0]->type==Constant &&
          nodes[1]->type==Constant &&
          nodes[2]->type==Constant){
        bfunc.exec( funcName,
                    nodes[0]->cvalue, nodes[0]->csize,
                    nodes[1]->cvalue, nodes[1]->csize,
                    nodes[2]->cvalue, nodes[2]->csize,
                    cvalue, csize );
        type = Constant;
        nodes.clear();
        } else {
        csize = bfunc.outSz( funcName,
                             nodes[0]->csize,
                             nodes[1]->csize,
                             nodes[2]->csize );
        }
      }
    }

  if( c && type==Operator ){
    type  = Constant;

    for( int i=0; i<4; ++i ){
      if( opType==Add )
        cvalue[i] = nodes[0]->cvalue[i] + nodes[1]->cvalue[i];
      if( opType==Sub )
        cvalue[i] = nodes[0]->cvalue[i] - nodes[1]->cvalue[i];
      if( opType==Mul )
        cvalue[i] = nodes[0]->cvalue[i] * nodes[1]->cvalue[i];
      if( opType==Div )
        cvalue[i] = nodes[0]->cvalue[i] / nodes[1]->cvalue[i];
      }

    for( int i=csize; i<4; ++i )
      cvalue[i] = 0;

    nodes.clear();
    }

  if( type==Vertex ){
    if( cx.decl->contains(vcomponent, vcomponentIndex) ){
      csize = componentSz(cx, vcomponent);
      } else {
      type  = Constant;
      csize = 1;
      std::fill( cvalue, cvalue+4, 0 );
      }
    }

  if( type==Constant && csize1==1){
    while( csize>1 && cvalue[csize-1]==0 )
      --csize;
    }
  }

void ShaderSource::preprocessR( ShaderSource::Context &cx,
                                bool isVs ) {
  if( !isSwizzle() && type==Math ){
    int sz[3] = {};
    argsSize( sz[0], sz[1], sz[2] );

    if( funcName=="mix" ){
      sz[0] = std::min(sz[0], csize);
      sz[1] = sz[0];
      sz[2] = std::min( sz[2], sz[0] );
      }

    for( size_t i=0; i<nodes.size(); ++i )
      if( nodes[i]->isSwizzle() ){
        nodes[i]->csize = std::min( nodes[i]->csize, sz[i] );
        }
    }

  if( type==Operator && opType==Mul ){
    for( size_t i=0; i<nodes.size(); ++i )
      if( nodes[i]->isSwizzle() ){
        nodes[i]->csize = std::min( nodes[i]->csize, csize );
        for( int r=csize; r<4; ++r )
          nodes[i]->mathSW[r] = '-';
        }
    }

  isVs |= shOutType==Transform;
  for( size_t i=0; i<nodes.size(); ++i ){
    nodes[i]->preprocessR( cx, isVs );
    }

  for( size_t i=0; i<nodes.size(); ++i ){
    isVarying  |= nodes[i]->isVarying;
    isUniform  |= nodes[i]->isUniform;
    isTexture  |= nodes[i]->isTexture;
    vInputUsed |= nodes[i]->vInputUsed;

    convStage = std::max(convStage, nodes[i]->convStage);
    }

  /*
  if( !isVs && opDefs.equal( *this, opDefs.getScreen, *cx.opt ) ){
    this->type = BuildIn;
    convStage  = std::max( convStage, fragmentStage );
    name       = "gl_FragCoord.xy";
    nodes.clear();
    }*/

  }

bool ShaderSource::isSwizzle() const {
  return type==Math && funcName.size()==0;
  }

bool ShaderSource::isMatrixOp() const {
  if( nodes.size()!=2 )
    return 0;

  return nodes[0]->csize1>1 || nodes[1]->csize1>1;
  }

void ShaderSource::fillNodes( ShaderSource::Context &v ) {
  for( size_t i=0; i<nodes.size(); ++i ){
    v.nodes.push_back( nodes[i] );
    nodes[i]->fillNodes(v);
    }
  }

void ShaderSource::optNodes(ShaderSource::Context &v) {
  bool ok = true;

  int id = 1;
  while( ok ){
    ok = false;
    for( size_t i=0; i<v.nodes.size(); ++i )
      if( !v.nodes[i]->hasTmp ){
        bool isTmp = false;
        for( size_t r=i+1; r<v.nodes.size(); ++r ){
          if( v.nodes[i]->nodes.size() &&
              *v.nodes[i]==*v.nodes[r]  ){
            isTmp = true;
            v.nodes[i]->hasTmp = id;
            v.nodes[r]->hasTmp = id;
            v.nodes[r] = v.nodes[i];
            ok = true;
            }
          }

        if( isTmp )
          ++id;
        }
    }

  for( size_t i=0; i<v.nodes.size(); ++i )
    if( v.nodes[i]->hasTmp ){
      bool isTmp = false;
      for( size_t r=0; r<v.nodes.size(); ++r ){
        if( i!=r &&//v.nodes[i].get()!=v.nodes[r].get() &&
            v.nodes[r]->hasTmp && *v.nodes[i]==*v.nodes[r]  ){
          isTmp = true;
          }
        }

      if( !isTmp )
        v.nodes[i]->hasTmp = 0;
      }

  for( size_t i=0; i<v.nodes.size(); ++i )
    if( v.nodes[i]->isSwizzle() ){
      v.nodes[i]->nodes[0]->hasTmp = id;
      ++id;
      }
  }

void ShaderSource::mkRState(ShaderSource::Context &v, Code &c ) {
  Tempest::RenderState &rs = c.rs;

  for( size_t i=0; i<nodes.size(); ++i )
    if( nodes[i]->shOutType==Lighting ){
      ShaderSource &cl = *nodes[i];

      if( opDefs.equal( cl, opDefs.addBlend, *v.opt ) ){
        rs.setBlend(1);
        rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::one,
                         Tempest::RenderState::AlphaBlendMode::one );

        if( opDefs.equal( *cl.nodes[0], opDefs.getScreen, *v.opt ) )
          nodes[i] = cl.nodes[1]; else
          nodes[i] = cl.nodes[0];

        nodes[i]->shOutType = Lighting;
        return;
        } else
      if( opDefs.equal( cl, opDefs.alphaBlend, *v.opt ) ){
        rs.setBlend(1);
        rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::src_alpha,
                         Tempest::RenderState::AlphaBlendMode::one_minus_src_alpha );

        std::shared_ptr<ShaderSource> ptr;
        ptr.reset( new ShaderSource() );
        ptr->type     = Math;
        ptr->funcName = "float4";
        ptr->csize    = 4;
        ptr->nodes.push_back( cl.nodes[0]->nodes[0] );
        ptr->nodes.push_back( cl.nodes[0]->nodes[1] );

        nodes[i] = ptr;
        nodes[i]->shOutType = Lighting;
        }
      }
  }

bool ShaderSource::mkVaryings( Context &cx,
                               bool forceDeepth ) {
  if( !forceDeepth && (!isTexture && isVarying && convStage<=varyingStage) ){
    bool ex = false;
    for( size_t i=0; i<cx.varying.size(); ++i )
      ex |= (cx.varying[i]==this);

    if( hasTmp )
      for( size_t i=0; i<cx.varying.size(); ++i )
          ex |= ( hasTmp==cx.varying[i]->hasTmp );

    if( !ex )
      cx.varying.push_back(this);
    return true;
    }

  bool dv = false;
  for( size_t i=0; i<nodes.size(); ++i ){
    dv |= nodes[i]->mkVaryings(cx, false);
    }

  return dv;
  }

bool ShaderSource::reduceVaryings( ShaderSource::Context &cx,
                                   size_t count ) {
  while( cx.varying.size()>count ){
    size_t n = -1, count = -1;
    for( size_t i=0; i<cx.varying.size(); ++i ){
      if( cx.varying[i]->vInputUsed.count()<count &&
          cx.varying[i]->type!=Vertex &&
          cx.varying[i]->shOutType==NonOut ){
        n     = i;
        count = cx.varying[i]->vInputUsed.count();
        }
      }

    if( n>cx.varying.size() )
      return false;

    auto s = cx.varying[n];
    cx.varying[n] = cx.varying.back();
    cx.varying.pop_back();
    s->convStage = fragmentStage;

    for( size_t i=0; i<s->nodes.size(); ++i ){
      bool ex = false;
      for( size_t r=0; r<cx.varying.size(); ++r )
        ex |= (cx.varying[r]==s->nodes[i].get());

      if( !ex ){
        cx.varying.push_back(s->nodes[i].get());
        /*
        if( s->nodes[i]->shOutType==NonOut )
          cx.varying.push_back(s->nodes[i].get()); else
          cx.varying.push_back( new ShaderSource(*s->nodes[i]) );
        */
        cx.varying.back()->shOutType=NonOut;
        cx.varying.back()->convStage = varyingStage;
        }
      }
    }

  return true;
  }

void ShaderSource::mkTransformVarying(ShaderSource::Context &cx) {
  for( size_t i=0; i<cx.varying.size(); ++i )
    if( cx.varying[i]->shOutType==Transform )
      return;

  shOutType = Transform;
  cx.varying.push_back(this);
  }

void ShaderSource::mkUniforms( ShaderSource::Context &cx,
                               bool forceDeepth) {
  if( !forceDeepth && (isTexture||isUniform) && uniformName.size()!=0 ){
    if( isTexture ){
      for( size_t i=0; i<nodes.size(); ++i ){
        nodes[i]->mkUniforms(cx, false);
        }
      }

    cx.uniform.push_back(this);
    return;
    }

  for( size_t i=0; i<nodes.size(); ++i ){
    nodes[i]->mkUniforms(cx, false);
    }
  }

std::string ShaderSource::compileOperator( const std::string & sep,
                                           Context &v,
                                           bool vshader,
                                           int vecSz ) const {
  if( nodes.size()!=2 ){
    return expand("0.0", 1, vecSz, v);
    }

  bool lmat = (nodes[0]->csize1>1),
       rmat = (nodes[1]->csize1>1);

  if( opType!=Mul && (lmat!=rmat) ){
    return expand("0.0", 1, vecSz, v);
    }

  if( opType==Mul && (lmat||rmat) ){
    if( v.lang==CompileOptions::Cg ){
      std::string str = "mul( ";
      for( size_t i=0; i<nodes.size(); ++i ){
        if( i )
          str += ", ";
        str += nodes[i]->src(sep, v, vshader, 4);
        }

      str += " )";
      if( vecSz!=4 ){
        str += swizle(vecSz);
        }
      return str;
      } else {
      std::string str = "( ";
      for( size_t i=0; i<nodes.size(); ++i ){
        if( i )
          str += "*";
        str += nodes[i]->src(sep, v, vshader, 4);
        }

      str += " )";
      if( vecSz!=4 ){
        str += swizle(vecSz);
        }
      return str;
      }
    }

  std::string str = "( ";

  static const char* sop[] = {
    "+", "-", "*", "/"
    };

  int nsz = outSize();
  if( opType==Add &&
      ( nodes[0]->isZero() || nodes[1]->isZero() ) ){
    if( !nodes[0]->isZero() )
      return nodes[0]->src(sep, v, vshader, vecSz); else
      return nodes[1]->src(sep, v, vshader, vecSz);
    } else
  if( opType==Sub &&
      ( nodes[0]->isZero() || nodes[1]->isZero() ) ){
    if( !nodes[0]->isZero() )
      return nodes[0]->src(sep, v, vshader, vecSz); else
      return "(-"+ nodes[1]->src(sep, v, vshader, vecSz)+")";
    } else {
    for( size_t i=0; i<nodes.size(); ++i ){
      if( i )
        str += sop[opType];
      str += nodes[i]->src(sep, v, vshader, nsz);
      }
    }

  str += " )";
  return expand(str, nsz, vecSz,v);
  }

std::string ShaderSource::compileMath( const std::string &sep,
                                       ShaderSource::Context &v,
                                       bool vshader,
                                       int vecSz) const {
  if( bfunc.argsCount(funcName)==0 ){
    int ssz = 1;
    for( int i=0; i<csize; ++i ){
      if( mathSW[i]=='w')
        ssz = 4; else
        ssz = std::max(ssz, mathSW[i]-'x'+1);
      }

    ssz = std::max(ssz, nodes[0]->csize);
    std::string s = nodes[0]->src(sep, v, vshader, ssz);

    if( nodes[0]->csize==1 ){
      std::string str = floatN(v,vecSz) + "(";

      for( int i=0; i<vecSz; ++i ){
        if( i )
          str +=", ";

        if( mathSW[i]!='x' )
          str += "0.0"; else
          str += s;
        }
      str += ")";
      return str;
      } else {
      s+=".";
      for( int i=0; i<4; ++i )
        if( mathSW[i]!='-' )
          s += mathSW[i];
      }

    return expand( s, csize, vecSz, v);
    }

  if( funcName=="saturate" && isGL(v) ){
    int sz = nodes[0]->csize;
    static const std::string oneF[] = {
      "1.0", "1.0, 1.0", "1.0, 1.0, 1.0", "1.0, 1.0, 1.0, 1.0"
      };

    std::string s =
        "clamp("+nodes[0]->src(sep, v, vshader, sz)+", "
        +expand("0.0", 1, sz, v ) +", "
        +floatN(v, sz) + "(" + oneF[sz-1] +") "
        +")";

    return expand( s, csize, vecSz, v);
    }

  if( funcName=="float4" ){
    static const std::string f[] = {
      "float4", "vec4", "vec4"
    };
    std::string s =
        f[v.lang] +"("
        +nodes[0]->src(sep, v, vshader, 3)+", "
        +nodes[1]->src(sep, v, vshader, 1)
        +")";

    return expand( s, csize, vecSz, v);
    }

  if( bfunc.argsCount(funcName)==1 ){
    std::string s = funcName +
        "("+nodes[0]->src(sep, v, vshader, nodes[0]->csize)+")";

    return expand( s, csize, vecSz, v);
    }

  int sz0 = 0, sz1 = 0, sz2 = 0;

  argsSize(sz0, sz1, sz2);

  if( bfunc.argsCount(funcName)==2 ){
    std::string s = funcName +
        "("+
        nodes[0]->src(sep, v, vshader, sz0)+
        ", " +
        nodes[1]->src(sep, v, vshader, sz1)+
        ")";

    return expand( s, csize, vecSz, v);
    }

  if( bfunc.argsCount(funcName)==3 ){
    std::string s = funcName +
        "("+
        nodes[0]->src(sep, v, vshader, sz0)+
        ", " +
        nodes[1]->src(sep, v, vshader, sz1)+
        ", " +
        nodes[2]->src(sep, v, vshader, sz2)+
        ")";

    return expand( s, csize, vecSz, v);
    }

  return "0.0";
  }

int ShaderSource::outSize() const {
  if( type==Math && bfunc.argsCount(funcName)==0 ){
    int s = 1;
    for( int i=1; i<4; ++i )
      if( mathSW[i]!='-' )
        s = i+1;
    return s;
    }

  if( type==Math && bfunc.argsCount(funcName)==1 ){
    if( nodes.size()==0 )
      return 1;
    return bfunc.outSz( funcName, nodes[0]->csize );
    }

  if( type==Math && bfunc.argsCount(funcName)==2 ){
    return bfunc.outSz( funcName, nodes[0]->csize, nodes[1]->csize );
    }

  if( type==Constant ){
    int s = 1;
    for( int i=1; i<csize; ++i )
      if( cvalue[i] )
        s = i+1;
    return s;
    }

  if( type==TextureRect || type==Texture || type==Vertex )
    return csize;

  if( type==Operator )
    return csize;

  int nsz = 0;
  for( size_t i=0; i<nodes.size(); ++i )
    nsz = std::max( nsz, nodes[i]->outSize() );

  return nsz;
  }

int ShaderSource::componentSz( const Context &cx,
                               Tempest::Usage::UsageType t ) const {
  return cx.decl->componentSize(t);
  }

bool ShaderSource::isZero() const {
  if( type!=Constant )
    return 0;

  for( int i=0; i<csize; ++i )
    if( cvalue[i] )
      return 0;

  return 1;
  }

bool ShaderSource::isOne() const {
  if( type!=Constant )
    return 0;

  for( int i=0; i<csize; ++i )
    if( cvalue[i]!=1 )
      return 0;

  return 1;
  }

void ShaderSource::setToZero() {
  type = Constant;
  csize  = 1;
  csize1 = 0;
  std::fill( cvalue, cvalue+4, 0 );
  nodes.clear();
  }

void ShaderSource::setToNode( int n ) {
  ShaderOuputType t = shOutType;
  ShaderSource tmp = *nodes[n];
  *this = tmp;
  shOutType = t;
  }

const std::string &ShaderSource::floatN( const Context &cx, int n) {
  return floatN(cx,n,1);
  }

const std::string &ShaderSource::floatN(const ShaderSource::Context &cx,
                                        int n, int m) {
  return floatN(cx.lang,n,m);
  }

std::string ShaderSource::to_string(int i) {
  static const std::string v10[] = {
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9"
    };

  if( i>=0 && i<=9 )
    return v10[i];

  std::stringstream ss;
  ss << i;
  return ss.str();
  }

void ShaderSource::argsSize(int &sz0, int &sz1, int &sz2) const {
  if( nodes.size()>=1 )
    sz0 = nodes[0]->csize;
  if( nodes.size()>=2 )
    sz1 = nodes[1]->csize;
  if( nodes.size()>=3 )
    sz2 = nodes[2]->csize;

  argsSize(sz0, sz1, sz2, funcName);
  }

void ShaderSource::argsSize( int &sz0, int &sz1, int &sz2,
                             const std::string &funcName ) {
  BuildInFunction::ArgsSize as = bfunc.argsSz(funcName);
  if( as==BuildInFunction::asMaximize ){
    int sz = std::max( std::max(sz0, sz1), sz2 );
    sz0 = sz;
    sz1 = sz;
    sz2 = sz;
    }

  if( as==BuildInFunction::asMinimaize ){
    int sz = sz0;

    if( sz1 )
      sz = std::min(sz, sz1);
    if( sz2 )
      sz = std::min(sz, sz2);

    sz0 = sz;
    sz1 = sz;
    sz2 = sz;
    }

  if( as==BuildInFunction::asSetTo3 ){
    sz0 = 3;
    sz1 = 3;
    sz2 = 3;
    }

  if( as==BuildInFunction::asSetTo4 ){
    sz0 = 4;
    sz1 = 4;
    sz2 = 4;
    }

  if( funcName=="mix" ){
    int sz = std::max(sz0,sz1);
    if( sz2!=1 ){
      sz = std::min(sz2, sz);
      sz2 = sz;
      }

    sz0 = sz;
    sz1 = sz;
    }
  }

const std::string &ShaderSource::floatN( Lang l, int n) {
  return floatN(l,n,1);
  }

const std::string &ShaderSource::floatN( Lang lang,
                                         int n, int m) {
  --n;
  --m;

  static const std::string cg[4][4] = {
    {"float",  "float1x2", "float1x3", "float1x4"},
    {"float2", "float2x2", "float2x3", "float2x4"},
    {"float3", "float3x2", "float3x3", "float3x4"},
    {"float4", "float4x2", "float4x3", "float4x4"}
    };

  static const std::string gl[4][4] = {
    {"float",  "mat1x2", "mat1x3", "mat1x4"},
    {"vec2",   "mat2",   "mat2x3", "mat2x4"},
    {"vec3",   "mat3x2", "mat3",   "mat3x4"},
    {"vec4",   "mat4x2", "mat4x3", "mat4"}
    };

  if( lang==CompileOptions::Cg )
    return cg[n][m]; else
    return gl[n][m];
  }

bool ShaderSource::isGL(const ShaderSource::Context &c) {
  return c.lang==CompileOptions::GLSL || c.lang==CompileOptions::GLSLES;
  }
