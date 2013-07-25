#include "shadersource.h"

#include <utility>
#include <sstream>
#include <map>
#include <unordered_set>

#include <Tempest/Model>
#include "objectcode.h"
#include "vertexinputassembly.h"

BuildInFunction ShaderSource::bfunc;
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
  }

ShaderSource::Code ShaderSource::code(Lang lang) {
  return code( Tempest::Model<>::declarator(), lang );
  }

ShaderSource::Code ShaderSource::code(const Tempest::VertexDeclaration::Declarator &d, Lang lang) {
  VertexInputAssembly v(d, lang);
  return code( v, lang );
  }

ShaderSource::Code ShaderSource::code(VertexInputAssembly &d, Lang lang) {
  Context cx;
  cx.decl = &d;
  cx.lang = lang;

  preprocess(cx);

  fillNodes(cx);

  {
    size_t id = 0;
    std::map<TextureSemantic, size_t> m;
    for( size_t i=0; i<cx.nodes.size(); ++i ){
      if( cx.nodes[i]->type==Texture ||
          cx.nodes[i]->type==TextureRect ){
        auto f = m.find( cx.nodes[i]->textureSemantic );

        if( f==m.end() ){
          cx.nodes[i]->uniformName = "texture"+std::to_string(id);
          m[ cx.nodes[i]->textureSemantic ] = id;
          ++id;
          } else {
          cx.nodes[i]->uniformName = "texture"+std::to_string(f->second);
          }
        }
      }
    }

  optNodes(cx);
  mkVaryings(cx, true);
  mkUniforms(cx, false);
  mkTransformVarying(cx);

  Code c;
  {
  std::string scode = color("\n  ", cx),
              tcode = compileTmp(cx,"\n  ", false);

  std::stringstream fs;
  fs << varying(cx) << std::endl;

  if( cx.lang==Cg ){
    fs << "struct FS_Output{" << std::endl
       << "  float4 color:COLOR;" << std::endl
       << "  };" << std::endl << std::endl;
    }

  if( cx.lang==Cg ){
    fs << "FS_Output main( VS_Output vs";
    fs << uniform(cx, false);
    } else {
    fs << uniform(cx, false) << std::endl;
    fs << "void main(){";
    }

  if( cx.lang==Cg ){
    fs << " ) {" << std::endl;
    fs << "  FS_Output fs;" << std::endl;
    } else {
    fs << std::endl;
    }

  fs << "  " << tcode << scode;

  if( cx.lang==Cg )
    fs << "return fs;\n  }" << std::endl; else
    fs << "}";

  c.fs = fs.str();
  }

  {
    std::stringstream ss;
    for( size_t i=0; i<cx.varying.size(); ++i ){
      if( cx.lang==Cg )
        ss << "vs.";

      ss << varyingName(cx.varying[i],cx, -1)
         << " = " << cx.varying[i]->src("\n  ", cx, true, 4) <<";\n\n  ";
      }

    std::stringstream vs;
    vs << decl(*cx.decl) << std::endl
       << varying(cx)    << std::endl;

    std::string tCode = compileTmp(cx,"\n  ", true);

    if( cx.lang==Cg ){
      vs << "VS_Output main( VS_Input vs_in";
      vs << uniform(cx, true) << " ) {" << std::endl;
      vs << "  VS_Output vs;" << std::endl << "  ";
      } else {
      vs << uniform(cx, true) << std::endl << std::endl;
      vs << "void main() {" << std::endl << "  ";
      }

    vs << tCode;
    vs << ss.str();
    if( cx.lang==Cg )
      vs << "return vs;" << std::endl <<"  ";
    vs << "}" << std::endl;

    c.vs = vs.str();
    }

  for( size_t i=0; i<cx.uniform.size(); ++i ){
    ShaderSource* s = cx.uniform[i];

    Code::Uniform u;
    u.name     = s->uniformName;
    u.fileName = s->fileName;

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

std::string ShaderSource::compileTmp( ShaderSource::Context &cx,
                                      const std::string &sep,
                                      bool vs ) const {
  //std::vector<bool> used;

  bool cc = true;
  std::string str;

  std::set<ShaderSource*> compiled;
  while( cc ){
    cc = false;
    std::stringstream ss;
    std::vector< std::shared_ptr<ShaderSource>> ptr;

    for( size_t i=0; i<cx.nodes.size(); ++i )
      if( cx.nodes[i]->tmpUsed )
        ptr.push_back( cx.nodes[i] );

    for( size_t i=0; i<ptr.size(); ++i )
      if( compiled.find( ptr[i].get())==compiled.end() ){
        ss << floatN(cx, ptr[i]->csize);

        ss << " "
           << "tmp"
           << ptr[i]->hasTmp
           << " = ";

        ptr[i]->hasTmp  = 0;
        ss << ptr[i]->src(sep, cx, vs, ptr[i]->csize) <<";\n" << sep;
        ptr[i]->tmpUsed = false;

        compiled.insert( ptr[i].get() );
        cc = true;
        }

    str = ss.str() + str;
    }

  return str;
  }

bool ShaderSource::operator ==(const ShaderSource &s) const {
  if( this==&s )
    return 1;

  if( nodes.size()!=s.nodes.size() )
    return 0;

  for( size_t i=0; i<nodes.size(); ++i )
    if( *nodes[i]!=*s.nodes[i] )
      return 0;

  return std::tie( s.type, s.isVarying, s.isUniform,
                   s.cvalue[0], s.cvalue[1], s.cvalue[2], s.cvalue[3],
                   s.csize, s.csize1,
                   s.opType, s.vcomponent,
                   //s.name,
                   s.uniformName,
                   s.textureSemantic, s.funcName,
                   s.convStage,
                   s.shOutType,
                   s.mathSW[0], s.mathSW[1], s.mathSW[2], s.mathSW[3] ) ==
        std::tie( type, isVarying, isUniform,
                  cvalue[0], cvalue[1], cvalue[2], cvalue[3],
                  csize, csize1,
                  opType, vcomponent,
                  //name,
                  uniformName,
                  textureSemantic, funcName,
                  convStage,
                  shOutType,
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
  if( oldS>sz )
    return s + swizle(sz);

  if( oldS==sz )
    return s;

  if( sz==1 )
    return s;

  std::stringstream ss;

  if( cx.lang==Cg )
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
                               const Context &cx,
                               bool vshader,
                               int vecSz ) const {
  std::string str;
  if( vshader )
    usedInVs = true; else
    usedInFs = true;

  if( hasTmp ){
    tmpUsed = true;
    return expand( "tmp" + std::to_string(hasTmp), csize, vecSz, cx );
    } else
  if( type==Texture || type==TextureRect ){
    if( nodes.size()==0 )
      return expand("0.0", 1, vecSz, cx);

    if( type==TextureRect && cx.lang==Cg ){
      str = "texRECTlod(";
      str += uniformName +", "+nodes[0]->src(sep, cx, vshader, 4) +" )";
      } else {
      str = cx.lang==Cg ? "tex2D( ":"texture2D(";
      str += uniformName +", "+nodes[0]->src(sep, cx, vshader, 2) +" )";
      }

    if( vecSz!=4 )
      str += swizle(vecSz);
    return str;
    } else
  if( !vshader && (convStage==varyingStage || convStage==vertexStage) ){
    if( cx.lang==Cg )
      return "vs."+varyingName(this, cx, vecSz); else
      return varyingName(this, cx, vecSz);
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

std::string ShaderSource::color(const std::string &sep,
                                const Context &v) const {
  static const std::string fr[] = {
    "fs.color = ",
    "gl_FragColor = "
    };

  for( size_t i=0; i<nodes.size(); ++i )
    if( nodes[i]->shOutType==Lighting ){
      return fr[v.lang] + nodes[i]->src(sep, v, false, 4) +";"+ sep;
      }

  if( v.lang==Cg )
    return fr[0]+"float4(0.0);"+sep; else
    return fr[1]+"vec4(0.0);"+sep;
  }

std::string ShaderSource::transform(const std::string &sep,
                                    const Context &v) const {
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

  return "TEXCOORD" + std::to_string(id);
  }

std::string ShaderSource::decl( VertexInputAssembly &d ) {
  return d.vsInput();
  }

std::string ShaderSource::varyingName( const ShaderSource *s,
                                       const Context &cx,
                                       int sz ) const {
  const std::vector<ShaderSource*>& v = cx.varying;
  int id=0, n = 0;
  for( size_t i=0; i<v.size(); ++i ){
    if( v[i]==s ){
      id = n;
      }

    if( v[i]->shOutType==Transform ){
      if( v[i]==s ){
        if( cx.lang==Cg )
          return "position"; else
          return "gl_Position";
        }
      } else {
      ++n;
      }
    }

  if( sz!=-1 && sz!=4 ){
    std::string v = "v" + std::to_string(id) + swizle(sz);
    return v;
    }
  return "v" + std::to_string(id);
  }

std::string ShaderSource::varying( Context &v ) const {
  std::stringstream ss;

  if( v.lang==Cg )
    ss << "struct VS_Output{\n";

  for( size_t i=0; i<v.varying.size(); ++i )
    if( v.lang==Cg || v.varying[i]->shOutType!=Transform ){
      if( v.lang==Cg )
        ss << "  ";
      if( v.lang==GLSL )
        ss << "varying ";

      ss << floatN(v,4) <<" ";
      ss << varyingName(v.varying[i], v, -1) ;

      if( v.lang==Cg )
        ss << " : " << varyingReg (v.varying[i], v);
      ss << ";\n";
      }

  if( v.lang==Cg )
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

      if( v.lang==Cg )
        ss << ",\n    uniform "; else
        ss << "uniform ";

      if( s->isTexture ){
        if( s->type==TextureRect && v.lang==Cg )
          ss << " sampler2DRect "; else
          ss << " sampler2D ";
        } else {
        ss << " " << floatN(v, s->csize, s->csize1) <<" ";
        }

      ss << v.uniform[i]->uniformName;

      if( v.lang==GLSL )
        ss <<";"<< std::endl;
      }
    }

  return ss.str();
  }

void ShaderSource::preprocess( Context & cx ) {
  int sz = 0;
  for( size_t i=0; i<nodes.size(); ++i )
    if( nodes[i]!=0 ){
      nodes[sz] = nodes[i];
      ++sz;
      }
  nodes.resize(sz);

  if( name=="Transform" )
    shOutType = Transform;

  if( name=="Lighting" )
    shOutType = Lighting;

  if( type==ExternLib ){
    ObjectCode c;
    c.wrkFolder = wrkFolder;
    ShaderOuputType t = shOutType;
    //c.loadFromFile( fileName.data() );
    std::wstring file = *c.wrkFolder+L"/lib/"+fileName;
    c.loadFromFile( file.data() );
    *this = *c.codeOf(0, nodes);
    shOutType = t;
    }

  bool c = true;
  if( shOutType==Transform ){
    isVarying  = true;
    }

  for( size_t i=0; i<nodes.size(); ++i ){
    nodes[i]->preprocess(cx);
    c &= ( nodes[i]->type==Constant );

    isVarying |= nodes[i]->isVarying;
    isUniform |= nodes[i]->isUniform;
    isTexture |= nodes[i]->isTexture;

    convStage = std::max(convStage, nodes[i]->convStage);
    }

  if( isTexture ){
    convStage = fragmentStage;
    if( nodes.size() && nodes[0]->isVarying )
      nodes[0]->convStage = std::max(varyingStage, nodes[0]->convStage);
    }

  if( isVarying )
    convStage = std::max( vertexStage, convStage );

  if( type==Texture || type==TextureRect ){
    csize = 4;
    }

  if( uniformName=="dxScreenOffset" && cx.lang!=Cg ){
    type = Constant;
    csize = 1;
    std::fill(cvalue, cvalue+4, 0.f);
    }

  if( type==Operator && nodes.size()!=2 ){
    type  = Constant;

    if( nodes.size()==1 ){
      ShaderOuputType t = shOutType;
      ShaderSource tmp = *nodes[0];
      *this = tmp;
      shOutType = t;
      } else {
      csize = 1;
      std::fill(cvalue, cvalue+4, 0.f);
      nodes.clear();
      }
    }

  if( type==Operator ){
    if( opType==Add || opType==Sub )
      for( size_t i=0; i<nodes.size(); ++i ){
        csize  = std::max( csize,  nodes[i]->csize );
        csize1 = std::max( csize1, nodes[i]->csize1 );
        }
    if( opType==Mul ){
      csize  = nodes[0]->csize;
      csize1 = nodes[0]->csize1;

      for( size_t i=0; i<nodes.size(); ++i ){
        csize  = std::min( csize,  nodes[i]->csize );
        csize1 = std::min( csize1, nodes[i]->csize1 );
        }
      }
    if( opType==Div )
      for( size_t i=0; i<nodes.size(); ++i ){
        csize  = nodes[0]->csize;
        csize1 = nodes[0]->csize1;
        }
    }

  if( type==Math && nodes.size()==0 ){
    type = Constant;
    std::fill(cvalue, cvalue+4, 0.f);
    }

  if( type==Math && bfunc.argsCount(funcName)==2 && nodes.size()<2 ){
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

      if( exp ){
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
          if(  v.nodes[i]->nodes.size() &&
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
  }

bool ShaderSource::mkVaryings( Context &cx,
                               bool forceDeepth ) {
  if( !forceDeepth && !isTexture && isVarying ){
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
                                           const Context &v,
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
    if( v.lang==Cg ){
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
                                       const ShaderSource::Context &v,
                                       bool vshader,
                                       int vecSz) const {
  if( bfunc.argsCount(funcName)==0 ){
    int ssz = 1;
    for( int i=0; i<csize; ++i ){
      if( mathSW[i]=='w')
        ssz = 4; else
        ssz = std::max(ssz, mathSW[i]-'x'+1);
      }

    std::string s = "("+nodes[0]->src(sep, v, vshader, ssz)+").";
    for( int i=0; i<4; ++i )
      if( mathSW[i]!='-' )
        s += mathSW[i];

    return expand( s, csize, vecSz, v);
    }

  if( bfunc.argsCount(funcName)==1 ){
    std::string s = funcName +
        "("+nodes[0]->src(sep, v, vshader, nodes[0]->csize)+")";

    //if( funcName=="length" )
      //return expand(s, 1, vecSz );

    return expand( s, csize, vecSz, v);
    }

  if( bfunc.argsCount(funcName)==2 ){
    int sz0 = nodes[0]->csize;
    int sz1 = nodes[1]->csize;

    BuildInFunction::ArgsSize as = bfunc.argsSz(funcName);
    if( as==BuildInFunction::asMaximize ){
      int sz = std::max( nodes[0]->csize,
                         nodes[1]->csize );
      sz0 = sz;
      sz1 = sz;
      }

    if( as==BuildInFunction::asMinimaize ){
      int sz = std::min( nodes[0]->csize,
                         nodes[1]->csize );
      sz0 = sz;
      sz1 = sz;
      }

    if( as==BuildInFunction::asSetTo3 ){
      sz0 = 3;
      sz1 = 3;
      }

    std::string s = funcName +
        "("+
        nodes[0]->src(sep, v, vshader, sz0)+
        ", " +
        nodes[1]->src(sep, v, vshader, sz1)+
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

const std::string &ShaderSource::floatN( const Context &cx, int n) {
  return floatN(cx,n,1);
  }

const std::string &ShaderSource::floatN(const ShaderSource::Context &cx,
                                        int n, int m) {
  return floatN(cx.lang,n,m);
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

  if( lang==Cg )
    return cg[n][m]; else
    return gl[n][m];
  }
