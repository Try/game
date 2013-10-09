#include "resource.h"

#include <Tempest/VertexShaderHolder>
#include <Tempest/LocalVertexBufferHolder>
#include <Tempest/LocalTexturesHolder>
#include <Tempest/Sprite>

#include <Tempest/SystemAPI>

#include "sound/sound.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>

#include <rapidjson/document.h>
#include "graphics/translate/objectcode.h"

Resource::Resource( Tempest::TextureHolder       & tx,
                    Tempest::LocalTexturesHolder & ltx,

                    Tempest::VertexBufferHolder &  vh,
                    Tempest::LocalVertexBufferHolder &lvh,
                    Tempest::IndexBufferHolder  &  ih,

                    Tempest::VertexShaderHolder &  vsh,
                    Tempest::FragmentShaderHolder& fsh )
  : texHolder(tx),
    ltexHolder(ltx),
    vboHolder(vh),
    lvboHolder(lvh),
    iboHolder(ih),
    vsHolder(vsh),
    fsHolder(fsh),
    pixmaps( ltexHolder ){
  models.owner   = this;
  textures.owner = this;
  vs.owner = this;
  fs.owner = this;
  px.owner = this;

  materials.owner = this;

  setupSettings(settings);

  Model model;

  model.loadMX( vboHolder, iboHolder, "data/models/util/cube.mx" );

  models  .add("null", model);
  Texture tex;
  tex.avg  = false;

  tex.data = texHolder.load("data/textures/w.png");
  textures.add("null",      tex );

  tex.data = texHolder.load("data/textures/norm.png");
  textures.add("null/norm", tex );
  //texturesAvg.add( "null", Tempest::Color() );

  //load( vs, "null", "data/sh/main_material.vert", "" );
  //load( fs, "null", "data/sh/main_material.frag", "" );
  }

const Model& Resource::model(const std::string &key) const {
  return models.get(key);
  }

const Model::Raw &Resource::rawModel(const std::string &key) const {
  if( !rawModels.contains(key ) ){
    const Model& m = models.get(key);

    rawModels.add( key, std::shared_ptr<Model::Raw>( m.getRaw() ) );
    }

  return *rawModels.get(key);
  }

Model Resource::model( const Tempest::Model<MVertex>::Raw &r ) const {
  Model m;

  m.load( lvboHolder, iboHolder, r, MVertex::decl() );

  return m;
  }

const Tempest::Texture2d &Resource::texture(const std::string &key) const {
  bool norm = false;
  if( key.size()>5 ){
    norm = true;
    const char* l = "/norm";

    for( int i=0; i<5; ++i )
      if( l[i]!=key[key.size()+i-5] )
        norm = false;
    }

  if( norm )
    return textures.get(key, textures.get("null/norm") ).data; else
    return textures.get(key, textures.get("null") ).data;
  }

Tempest::Texture2d &Resource::texture(const std::string &key) {
  bool norm = false;
  if( key.size()>5 ){
    norm = true;
    const char* l = "/norm";

    for( int i=0; i<5; ++i )
      if( l[i]!=key[key.size()+i-5] )
        norm = false;
    }

  if( norm )
    return textures.get(key, textures.get("null/norm") ).data; else
    return textures.get(key, textures.get("null") ).data;
  }

Tempest::Color Resource::textureAVG(const std::string &key) const {
  return textures.get(key).color;
  //return texturesAvg.get(key);
  }

bool Resource::findTexture(const std::string &key) {
  return textures.contains(key);
  }

Sound &Resource::sound(const std::string &key) const {
  return *sounds.get(key);
  }

const ObjectCode &Resource::material(const std::string &key) const {
  return *materials.get(key);
  }

const std::vector<std::string> &Resource::unitsMaterials() const {
  return umaterials;
  }

Tempest::VertexShader &Resource::vshader(const std::string &key) {
  VShader &s = vs.get(key);

  if( s.settings!=settingsStr ){
    s.settings = settingsStr;
    s.sh       = vsHolder.loadFromSource( s.settings +"\n" + s.def + s.src );
    }

  return s.sh;
  }

Tempest::FragmentShader &Resource::fshader(const std::string &key){
  FShader &s = fs.get(key);

  if( s.settings!=settingsStr ){
    s.settings = settingsStr;
    s.sh       = fsHolder.loadFromSource( s.settings +"\n"+ s.def + s.src );
    }

  return s.sh;
  }

Tempest::Sprite Resource::pixmap(const std::string &key) {
  //pixmaps.flush();
  return px.get(key);
  }

Tempest::Sprite Resource::pixmap( const Tempest::Pixmap &px,
                                  bool flush ) {
  Tempest::Sprite tmp = pixmaps.load(px);

  if( flush )
    pixmaps.flush();

  return tmp;
  }

Tempest::SpritesHolder &Resource::sprites() {
  return pixmaps;
  }

void Resource::flushPixmaps() {
  pixmaps.flush();
  }

void Resource::setupSettings( const GraphicsSettingsWidget::Settings &s ) {
  settings = s;
  std::ostringstream ss;

#ifdef __ANDROID__
  ss << "precision highp float;\n";
  ss << "#define oes_render\n";
#endif

#ifndef __ANDROID__
  if( s.api==GraphicsSettingsWidget::Settings::openGL )
#endif
    ss << "#define opengl\n";

  ss << "#define settings_oclusionenable "  << (s.oclusion>0? 1:0)        << std::endl
     << "#define settings_shadowenable "    << (s.shadowMapRes>0? 1:0)    << std::endl
     << "#define settings_shadowmapres "    <<  s.shadowMapRes            << std::endl
     << "#define settings_shadowmapfilter " <<  s.shadowFilterQ           << std::endl
     << "#define settings_glow "            << (s.glow? 1:0)              << std::endl
     << "#define settings_bloom "           << (s.bloom?1:0)              << std::endl
     << "#define settings_normalMap "       << (s.normalMap?1:0)          << std::endl
     << "#define settings_shadowTextures "  << (s.shadowTextures?1:0)     << std::endl
     << "#define settings_shadowenable_cl " << (s.transcurentShadows?1:0) << std::endl;

  settingsStr = ss.str();
  }

void Resource::load( Box<Model> &m,
                     const std::string &k,
                     const std::string &f,
                     bool ){
  auto it = m.loaded.find(f);

  if( it!=m.loaded.end() ){
    m.add(k, m.get( it->second ) );
    } else {
    Model model;
    model.loadMX( vboHolder, iboHolder, f );

    m.add(k, model );
    m.loaded[f] = k;
    }
  }

void Resource::load( Box<  std::shared_ptr<Model::Raw>  > &,
                     const std::string &,
                     const std::string &,
                     bool ){
  T_ASSERT(0);
  }

void Resource::load(const Box<std::shared_ptr<ObjectCode> > & ,
                    const std::string &k,
                    const Box< std::shared_ptr<ObjectCode> >::ToLoadPromision &p ){
  auto it = materials.loaded.find(p.fname);

  if( it!=materials.loaded.end() ){
    materials.add(k, materials.get( it->second ) );
    } else {
    std::shared_ptr<ObjectCode> s;
    s.reset( new ObjectCode() );
    static const std::shared_ptr<std::wstring> wrk =
        std::make_shared<std::wstring>(L"data/sh/material");

    s->wrkFolder = wrk;
    {
    s->loadFromFile( p.fname.data() );

    //MxAssembly assemb( MVertex::decl(), lang() );
    //mat.install( *co.codeOf(), assemb );
    }
    //s->load(f);

    materials.add(k,  s );
    materials.loaded[k] = p.fname;
    }
  }

void Resource::load( Box<VShader> &,
                     const std::string &,
                     const std::string &,
                     bool ){
  T_ASSERT(0);
  }

void Resource::load( Box<FShader> &,
                     const std::string &,
                     const std::string &,
                     bool  ){
  T_ASSERT(0);
  }

void Resource::load(Box<Tempest::Sprite> &,
                    const std::string &,
                    const std::string &,
                    bool ) {
  T_ASSERT(0);
  }

void Resource::load( const Tempest::Sprite& p, const std::string &f) {
  px.add( f, p );
  }

void Resource::load( Box< std::shared_ptr<Sound> >& sounds,
                     const std::string &k,
                     const std::string &f,
                     bool ){
  auto it = sounds.loaded.find(f);

  if( it!=sounds.loaded.end() ){
    sounds.add(k, sounds.get( it->second ) );
    } else {
    std::shared_ptr<Sound> s;
    s.reset( new Sound() );
    s->load(f);

    sounds.add(k,  s );
    sounds.loaded[f] = k;
    }
  }

void Resource::load( Box<std::shared_ptr<ObjectCode> > &,
                     const std::string &,
                     const std::string &,
                     bool ) {
  T_ASSERT(0);
  }

void Resource::load( Box<Texture> &textures,
                     const std::string &k,
                     const std::string &f,
                     bool avg ){
  auto it = textures.loaded.find(f);

  if( it!=textures.loaded.end() ){
    textures.add(k, textures.get( it->second ) );
    } else {
    Texture tex;
    tex.avg  = avg;

    if( avg ){
      Tempest::Pixmap pm(f);
      int cl[4] = {};
      for( int i=0; i<pm.width(); ++i )
        for( int r=0; r<pm.height(); ++r ){
          Tempest::Pixmap::Pixel px = pm.at(i,r);
          cl[0] += px.r;
          cl[1] += px.g;
          cl[2] += px.b;
          cl[3] += px.a;
          }

      if( pm.width()!=0 && pm.height()!=0 ){
        for( int i=0; i<4; ++i )
          cl[i] /= (pm.width()*pm.height());
        }

      tex.color  = Tempest::Color( 0.75*cl[0]/255.0,
                                   0.75*cl[1]/255.0,
                                   0.75*cl[2]/255.0,
                                   0.75*cl[3]/255.0 );

      tex.data = texHolder.load(f);
      } else {
      tex.data = texHolder.load(f);
      }

    textures.add(k, tex );
    textures.loaded[f] = k;
    }

  }

void Resource::load(Box<VShader> &vs,
                     const std::string &k,
                     const std::string &f,
                     const std::string &def , bool) {
  VShader sh;
  sh.def = def;
  sh.src = loadSrc(f);

  sh.settings = "! notloaded\n";
  vs.add(k, sh );
  }

void Resource::load( Box<FShader> &fs,
                     const std::string &k,
                     const std::string &f,
                     const std::string &def,
                     bool ) {
  FShader sh;
  sh.def = def;
  sh.src = loadSrc(f);
  sh.settings = "! notloaded\n";

  fs.add(k, sh );
  }

std::string Resource::loadSrc(const std::string &f) {
  return Tempest::SystemAPI::loadText( f.data() );
  }

void Resource::readPack(const rapidjson::Value &v) {
  if( v.IsArray() ){
    for( size_t i=0; i<v.Size(); ++i )
      if( v[i].IsObject() )
        readPack(v[i]);
    return;
    }

  if( !v.IsObject() )
    return;

  if( !v["name"].IsString() ){
    return;
    }

  const std::string name = v["name"].GetString();

  if( v["model"].IsString() ){
    models.preload( name+"/model", v["model"].GetString() );
    }

  if( v["diff"].IsString() ){
    textures.preload( name+"/diff", v["diff"].GetString(), false );
    }

  if( v["norm"].IsString() ){
    textures.preload( name+"/norm", v["norm"].GetString(), false );
    }

  if( v["glow"].IsString() ){
    textures.preload( name+"/glow", v["glow"].GetString(), false );
    }
  }

void Resource::load(const std::string &s) {
  using namespace rapidjson;
  std::string jsonstr = Tempest::SystemAPI::loadText(s.data());

  Document d;
  d.Parse<0>(jsonstr.data());

  if( d.HasParseError() ){
    size_t pe = d.GetErrorOffset(), pl = 1, pch = 0;
    for( size_t i=0; i<pe && i<jsonstr.size(); ++i ){
      pch++;
      if( jsonstr[i]=='\n' ){
        pch = 0;
        ++pl;
        }
      }

    std::cout << "[" << s <<", " << pl <<":" <<pch <<"]: ";
    std::cout << d.GetParseError() << std::endl;
    return;
    }

  const Value &data = d["data"];

  if( data.IsObject() ){
    const Value& model = data["model"];
    if( model.IsArray() ){
      for( size_t i=0; i<model.Size(); ++i ){
        const Value& v = model[i];
        if( v.IsObject() &&
            v["file"].IsString() &&
            v["name"].IsString() ){
          models.preload( v["name"].GetString(), v["file"].GetString() );
          }
        }
      }

    const Value& texture = data["texture"];
    if( texture.IsArray() ){
      for( size_t i=0; i<texture.Size(); ++i ){
        const Value& v = texture[i];
        if( v.IsObject() &&
            v["file"].IsString() &&
            v["name"].IsString() ){
          textures.preload( v["name"].GetString(), v["file"].GetString(), false );
          }
        }
      }

    const Value& texture_with_avg = data["texture_with_avg"];
    if( texture_with_avg.IsArray() ){
      for( size_t i=0; i<texture_with_avg.Size(); ++i ){
        const Value& v = texture_with_avg[i];
        if( v.IsObject() &&
            v["file"].IsString() &&
            v["name"].IsString() ){
          textures.preload( v["name"].GetString(), v["file"].GetString(), true );
          }
        }
      }

    const Value& pixmap = data["pixmap"];
    if( pixmap.IsArray() ){
      for( size_t i=0; i<pixmap.Size(); ++i ){
        const Value& v = pixmap[i];
        if( v.IsObject() &&
            v["src"].IsString() &&
            v["name"].IsString() ){
          std::string file = v["src"].GetString();
          load( pixmaps.load( Tempest::Pixmap(file) ), v["name"].GetString() );
          }
        }
      }

    const Value& sound = data["sound"];
    if( sound.IsArray() ){
      for( size_t i=0; i<sound.Size(); ++i ){
        const Value& v = sound[i];
        if( v.IsObject() &&
            v["file"].IsString() &&
            v["name"].IsString() ){
          sounds.preload( v["name"].GetString(), v["file"].GetString() );
          }
        }
      }

    const Value& material = data["material"];
    if( material.IsArray() ){
      for( size_t i=0; i<material.Size(); ++i ){
        const Value& v = material[i];
        if( v.IsObject() && v["file"].IsString() ){
          if( v["name"].IsString() ){
            materials.preload( v["name"].GetString(), v["file"].GetString() );
            }

          if( v["usage"].IsString() ){
            static const std::string
                //pefect   = "efect",
                material = "material";
            const char* u = v["usage"].GetString();
            if( u==material )
              umaterials.push_back( v["name"].GetString() );
            }

          if( v["z_writing"].IsBool() ){

            }
          }
        }
      }

    const Value& pack = data["pack"];
    if( pack.IsArray() ){
      for( size_t i=0; i<pack.Size(); ++i ){
        const Value& v = pack[i];
        readPack(v);
        }
      }

    const Value& module = data["module"];
    if( module.IsArray() ){
      for( size_t i=0; i<module.Size(); ++i ){
        const Value& v = module[i];
        if( v.IsObject() &&
            v["file"].IsString() ){
          load( v["file"].GetString() );
          }
        }
      }

    const Value& shaders = data["shaders"];
    if( shaders.IsArray() ){
      for( size_t i=0; i<shaders.Size(); ++i ){
        const Value& v = shaders[i];
        std::string def;
        if( v["define"].IsArray() ){
          const Value& d = v["define"];

          for( size_t i=0; i<d.Size(); ++i ){
            if( d[i].IsObject() &&
                d[i].MemberBegin()+1==d[i].MemberEnd() ){
              const Value& n = d[i].MemberBegin()->name,
                         & v = d[i].MemberBegin()->value;
              def += "#define ";
              def += n.GetString();

              if( v.IsString() ){
                def += " ";
                def += v.GetString();
                }
              def += "\n";
              }
            }
          }
        def += "#line 0\n";

        if( v["name"].IsString() &&
            v["vs"].IsString()   &&
            v["fs"].IsString() ){
          const char* name = v["name"].GetString();

          load(vs, name, v["vs"].GetString(), def, false );
          load(fs, name, v["fs"].GetString(), def, false );
          }
        }
      }

    }
  }
