#include "objectcode.h"

#include <rapidjson/document.h>
#include <Tempest/SystemAPI>

#include "shadersource.h"
#include <algorithm>

static void stype( ShaderSource & sh,
                   const std::string & s ){
  static struct {
    ShaderSource::Type t;
    std::string name;
  } types [] = {
    {ShaderSource::Unknown,      "block"},
    {ShaderSource::Constant,     "const"},
    {ShaderSource::Operator,     "operator"},
    {ShaderSource::Texture,      "texture"},
    {ShaderSource::TextureRect,  "texture_rect"},
    {ShaderSource::Vertex,       "vertex"},
    {ShaderSource::LibConnect,   "libOutput"},
    {ShaderSource::Math,         "math"},
    {ShaderSource::Math,         "func1arg"},
    {ShaderSource::Math,         "func2arg"},
    {ShaderSource::ExternLib,    "func"},
    {ShaderSource::ShaderOutput, "output"},
    {ShaderSource::Unknown, ""}
  };

  sh.type = ShaderSource::Unknown;
  for( size_t i=0; types[i].name.size(); ++i ){
    if( types[i].name==s )
      sh.type = types[i].t;
    }

  if( s=="input" ){
    sh.type       = ShaderSource::Vertex;
    sh.isVarying  = true;
    sh.vcomponent = ShaderSource::VertexComponent(1);
    sh.csize      = 4;
    }

  if( s=="mvp" ){
    sh.isUniform = true;
    sh.csize  = 4;
    sh.csize1 = 4;

    sh.uniformName = "mvpMatrix";
    }

  if( s=="object" ){
    sh.isUniform = true;
    sh.csize  = 4;
    sh.csize1 = 4;

    sh.uniformName = "objMatrix";
    }

  if( s=="time" ){
    sh.isUniform = true;
    sh.csize  = 1;
    sh.csize1 = 0;

    sh.uniformName = "time";
    }
  }

const std::string ObjectCode::texSemantic[] = {
  "Diffuse",
  "Normal",
  "Specular",
  "Emission",
  "Envi",
  "AblimientOclusion",
  "AlphaMask",
  ""
  };

ObjectCode::ObjectCode() {
  }

void ObjectCode::loadFromFile(const wchar_t *f) {
  load( Tempest::SystemAPI::loadText(f).data() );
  }

void ObjectCode::load(const char *data) {
  using namespace rapidjson;

  Document d;
  d.Parse<0>( data );

  if( d.HasParseError() )
    return;

  const Value &v = d["blocks"];
  units.clear();
  connect.clear();
  input.clear();
  output.clear();

  if( v.IsArray() ){
    for( size_t i=0; i<v.Size(); ++i ){
      const Value& obj = v[i];
      ShaderSource *b = new ShaderSource();
      std::shared_ptr<ShaderSource> sb = std::shared_ptr<ShaderSource>(b);

      if( obj["type"].IsString() ){
        std::string t = obj["type"].GetString();
        stype( *b, t.data() );

        if( t=="input" )
          input.push_back(i);
        if( t=="libOutput" )
          output.push_back(i);
        }

      if( obj["const"].IsArray() ){
        b->csize = std::min<int>( obj["const"].Size(), 4 );
        for( int i=0; i<b->csize; ++i )
          if( obj["const"][i].IsNumber() )
            b->cvalue[i] = obj["const"][i].GetDouble();
        }

      if( obj["operation"].IsString() &&
          obj["operation"].GetStringLength()==1 ){
        char op[] = {'+', '-', '*', '/'};
        for( int i=0; i<4; ++i )
          if( op[i]==obj["operation"].GetString()[0] )
            b->opType = ShaderSource::OperatorType(i);
        }

      if( obj["input"].IsInt() &&
          obj["input"].GetInt()>=1 ){
        b->vcomponent   = vcomponent( obj["input"].GetInt()-1 );
        b->inputConnect = 0;
        b->isVarying    = true;
        b->csize        = 4;
        }
      if( obj["input"].IsObject() ){
        const Value &v = obj["input"];

        if( v["vcomponent"].IsInt() )
          b->vcomponent = vcomponent( v["vcomponent"].GetInt() );

        if( v["connect"].IsInt() )
          b->inputConnect = Type( v["connect"].GetInt() );

        if( v["inputName"].IsString() )
          b->name = v["inputName"].GetString();

        b->isVarying  = true;
        b->csize      = 4;
        }

      {
        const Value &vx = obj["swizle"];
        if( vx.IsString() ){
          const char* c = vx.GetString();
          int sz = vx.GetStringLength();

          for( int i=0; i<sz && i<4; ++i )
            b->mathSW[i] = c[i];
          }
        }

      if( obj["function"].IsObject() ){
        const Value &vx = obj["function"]["fileName"];
        if( vx.IsString() ){
          b->fileName.assign( vx.GetString(), vx.GetString()+vx.GetStringLength() );
          }

        const Value &vy = obj["function"]["name"];
        if( vy.IsString() ){
          b->name     = vy.GetString();
          b->funcName = b->name;

          if( b->fileName.size()==0 )
            b->type     = ShaderSource::Math;
          }
        }

      if( obj["texture"].IsObject() ){
        const Value &vx = obj["texture"]["fileName"];
        if( vx.IsString() ){
          b->fileName.assign( vx.GetString(), vx.GetString()+vx.GetStringLength() );
          }

        const Value &vy = obj["texture"]["textureSemantic"];
        if( vy.IsString() ){
          b->textureSemantic = tsFromString( vy.GetString() );
          }

        b->isTexture = true;
        }

      if( obj["texture_rect"].IsObject() ){
        const Value &vx = obj["texture_rect"]["fileName"];
        if( vx.IsString() ){
          b->fileName.assign( vx.GetString(), vx.GetString()+vx.GetStringLength() );
          }

        const Value &vy = obj["texture_rect"]["textureSemantic"];
        if( vy.IsString() ){
          b->textureSemantic = tsFromString( vy.GetString() );
          }

        b->isTexture = true;
        b->type = ShaderSource::TextureRect;
        }

      if( obj["uniform"].IsObject() ){
        const Value &vx = obj["uniform"]["name"];
        if( vx.IsString() ){
          b->uniformName.assign( vx.GetString(), vx.GetString()+vx.GetStringLength() );
          }

        if( obj["uniform"]["size0"].IsInt() )
          b->csize  = obj["uniform"]["size0"].GetInt();

        if( obj["uniform"]["size1"].IsInt() )
          b->csize1 = obj["uniform"]["size1"].GetInt();

        b->isUniform = true;
        }

      //b->load(obj);
      sb->wrkFolder = wrkFolder;
      units.push_back( sb );
      }
    }

  const Value &p = d["connections"];
  if( p.IsArray() ){
    for( size_t i=0; i<p.Size(); ++i ){
      const Value& obj = p[i];

      Connect c;
      c.port1   = obj["port1"].GetInt();
      c.port2   = obj["port2"].GetInt();
      c.port1Id = obj["portId1"].GetInt();
      c.port2Id = obj["portId2"].GetInt();

      if( (c.port1)<units.size() &&
          (c.port2)<units.size() ){
        connect.push_back(c);
        }
      }
    }
  }

std::shared_ptr<ShaderSource> ObjectCode::codeOf(int outPort) {
  std::vector< std::shared_ptr<ShaderSource> > in;
  return codeOf(outPort, in);
  }

std::shared_ptr<ShaderSource>
  ObjectCode::codeOf( int outPort,
                      std::vector< std::shared_ptr<ShaderSource> > &in ) {
  std::vector< std::shared_ptr<ShaderSource> > src;
  src.resize( units.size() );

  for( size_t i=0; i<units.size(); ++i ){
    int inId = units[i]->inputConnect;

    if( inId>=0 && inId<int(in.size()) ){
      src[i] = in[ inId ];
      } else {
      src[i].reset( new ShaderSource( *units[i] ) );
      }
    }

  for( size_t i=0; i<connect.size(); ++i ){
    const Connect& c = connect[i];
    if( src[ c.port1 ]->nodes.size()<=c.port1Id-1 )
      src[ c.port1 ]->nodes.resize( c.port1Id );

    src[ c.port1 ]->nodes[c.port1Id-1] = src[c.port2];
    }

  for( size_t i=0; i<src.size(); ++i )
    if( src[i]->type==ShaderSource::ShaderOutput ){
      ShaderSource::ShaderOuputType t[] = {
        ShaderSource::Transform,
        ShaderSource::Lighting
        };

      for( size_t r=0; r<src[i]->nodes.size(); ++r )
        if( r<2 )
          src[i]->nodes[r]->shOutType = t[r];
      }

  if( outPort<0 ){
    for( size_t i=0; i<src.size(); ++i )
      if( src[i]->type==ShaderSource::ShaderOutput )
        return src[i];

    return std::make_shared<ShaderSource>();
    }

  if( outPort<int(output.size()) &&
      output[outPort]<src.size() &&
      src[ output[outPort] ]->nodes.size() )
    return src[ output[outPort] ]->nodes[0];

  return std::make_shared<ShaderSource>();
  }

Tempest::Usage::UsageType ObjectCode::vcomponent(int v) {
  static const Tempest::Usage::UsageType t[] = {
    Tempest::Usage::Position,
    Tempest::Usage::TexCoord,
    Tempest::Usage::Normal,
    Tempest::Usage::BiNormal,
    Tempest::Usage::Color
    };

  return t[v];
  }

ShaderSource::TextureSemantic ObjectCode::tsFromString(const char *ts) {
  for( size_t i=0; texSemantic[i].size(); ++i )
    if( texSemantic[i]==ts )
      return ShaderSource::TextureSemantic(i);

  return ShaderSource::tsDiffuse;
  }

const std::string &ObjectCode::toString(ShaderSource::TextureSemantic t) {
  return texSemantic[ int(t) ];
  }
