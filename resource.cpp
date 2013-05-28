#include "resource.h"

#include <Tempest/VertexShaderHolder>
#include <Tempest/LocalVertexBufferHolder>
#include <Tempest/LocalTexturesHolder>

#include <Tempest/SystemAPI>

#include "sound/sound.h"

#include <tinyxml.h>
#include <iostream>
#include <fstream>
#include <cmath>

struct Resource::XML{
  template< class Box, class ... Args >
  static void loadFile( Box & b, Resource &, TiXmlElement* pElement ){
    if ( !pElement )
      return;

    std::string file, name;

    for( TiXmlAttribute* pAttrib=pElement->FirstAttribute();
         pAttrib;
         pAttrib=pAttrib->Next() ) {
      std::string n = pAttrib->Name();

      if( n=="file" )
        file = pAttrib->Value();

      if( n=="name" )
        name = pAttrib->Value();
      }

    b.preload(name, file);
    //r.load(b, name, file );
    }

  template< class Box, class ... Args >
  static void loadFile( Box & b, Resource &, TiXmlElement* pElement, bool avg ){
    if ( !pElement )
      return;

    std::string file, name;

    for( TiXmlAttribute* pAttrib=pElement->FirstAttribute();
         pAttrib;
         pAttrib=pAttrib->Next() ) {
      std::string n = pAttrib->Name();

      if( n=="file" )
        file = pAttrib->Value();

      if( n=="name" )
        name = pAttrib->Value();
      }

    b.preload(name, file, avg);
    }

  template< class Box, class Box1 >
  static void loadFile( Box & b, Box1 & b1, Resource &r, TiXmlElement* pElement ){
    if ( !pElement )
      return;

    std::string file1, file, name;

    for( TiXmlAttribute* pAttrib=pElement->FirstAttribute();
         pAttrib;
         pAttrib=pAttrib->Next() ) {
      std::string n = pAttrib->Name();

      if( n=="vs" )
        file = pAttrib->Value();

      if( n=="fs" )
        file1 = pAttrib->Value();

      if( n=="name" )
        name = pAttrib->Value();
      }

    std::string def;

    for ( TiXmlNode* tg = pElement->FirstChild(); bool(tg);
          tg = tg->NextSibling() ){

      if ( tg->Type()==TiXmlNode::TINYXML_ELEMENT ) {
        std::string n = tg->Value();

        if( n=="define" ){
          for( TiXmlAttribute* pAttrib=tg->ToElement()->FirstAttribute();
               pAttrib;
               pAttrib=pAttrib->Next() ) {
            def += "#define ";
            def += pAttrib->Name();
            def += " ";
            def += pAttrib->Value();
            def += "\n";
            }
          }
        }
      }
    def += "#line 0\n";

    //std::cout << def << std::endl;
    r.load(b,  name, file,  def, false );
    r.load(b1, name, file1, def, false );
    }

  static void loadFile( PixmapsPool & pm, Resource &r, TiXmlElement* pElement ){
    if ( !pElement )
      return;

    std::string file, name;

    for( TiXmlAttribute* pAttrib=pElement->FirstAttribute();
         pAttrib;
         pAttrib=pAttrib->Next() ) {
      std::string n = pAttrib->Name();

      if( n=="src" )
        file = pAttrib->Value();

      if( n=="name" )
        name = pAttrib->Value();
      }

    r.load( pm.add( Tempest::Pixmap(file) ), name );
    }
  };

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
    Model::Raw  *model = new Model::Raw();

    model->index. resize( m.indexes().size()  );
    model->vertex.resize( m.vertexes().size() );

    m.vertexes().get( model->vertex.begin(),
                      model->vertex.end(),
                      0 );
    m.indexes().get( model->index.begin(),
                     model->index.end(),
                     0 );
    rawModels.add( key, std::shared_ptr<Model::Raw>(model) );
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

PixmapsPool::TexturePtr Resource::pixmap(const std::string &key) {
  //pixmaps.flush();
  return px.get(key);
  }

PixmapsPool::TexturePtr Resource::pixmap( const Tempest::Pixmap &px,
                                          bool flush ) {
  PixmapsPool::TexturePtr tmp = pixmaps.add(px);

  if( flush )
    pixmaps.flush();

  return tmp;
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

  ss << "#define settings_oclusionenable "  << (s.oclusion>0? 1:0) << std::endl
     << "#define settings_shadowenable "    << (s.shadowMapRes>0? 1:0) << std::endl
     << "#define settings_shadowmapres "    <<  s.shadowMapRes         << std::endl
     << "#define settings_shadowmapfilter " <<  s.shadowFilterQ        << std::endl
     << "#define settings_glow "            << (s.glow? 1:0)           << std::endl
     << "#define settings_bloom "           << (s.bloom?1:0)           << std::endl
     << "#define settings_normalMap "       << (s.normalMap?1:0)       << std::endl;

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
  assert(0);
  }

/*
void Resource::load( Box< Tempest::Color  > &,
                     const std::string &,
                     const std::string & ){
  assert(0);
  }*/

void Resource::load( Box<VShader> &,
                     const std::string &,
                     const std::string &,
                     bool ){
  assert(0);
  }

void Resource::load( Box<FShader> &,
                     const std::string &,
                     const std::string &,
                     bool  ){
  assert(0);
  }

void Resource::load(Box<PixmapsPool::TexturePtr> &,
                    const std::string &,
                    const std::string &,
                    bool ) {
  assert(0);
  }

void Resource::load(PixmapsPool::TexturePtr p, const std::string &f) {
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

      tex.color  = Tempest::Color( cl[0]/255.0,
                                   cl[1]/255.0,
                                   cl[2]/255.0,
                                   cl[3]/255.0 );

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

void Resource::readElement( TiXmlNode *node ) {
  if ( !node )
    return;

  if ( node->Type()==TiXmlNode::TINYXML_ELEMENT ) {
    std::string type = node->Value();
    if( type=="module"){
      std::string tmp;
      if( find( node->ToElement(), "file", tmp ) ){
        load(tmp);
        }
      } else

    if( type=="model"){
      XML::loadFile( models, *this, node->ToElement() );
      } else

    if( type=="texture"){
      XML::loadFile( textures, *this, node->ToElement(), false );
      } else

    if( type=="texture_with_avg"){
      XML::loadFile( textures, *this, node->ToElement(), true );
      } else

    if( type=="shader"){
      XML::loadFile( vs, fs, *this, node->ToElement() );
      } else

    if( type=="pixmap"){
      XML::loadFile( pixmaps, *this, node->ToElement() );
      } else

    if( type=="sound"){
      XML::loadFile( sounds, *this, node->ToElement() );
      } else

    dumpAttribs(node->ToElement() );
    }
/*
  for ( TiXmlNode* n = node->FirstChild(); n != 0; n = n->NextSibling() ){
    readElement( n );
    }
    */
}

std::string Resource::loadSrc(const std::string &f) {
  return Tempest::SystemAPI::loadText( f.data() );
  }
