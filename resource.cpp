#include "resource.h"

#include <Tempest/VertexShaderHolder>
#include <Tempest/LocalVertexBufferHolder>
#include <Tempest/LocalTexturesHolder>

#include <Tempest/AbstractSystemAPI>

#include "sound/sound.h"

#include <tinyxml.h>
#include <iostream>
#include <fstream>
#include <cmath>

struct Resource::XML{
  template< class Box, class ... Args >
  static void loadFile( Box & b, Resource &r, TiXmlElement* pElement ){
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
  static void loadFile( Box & b, Resource &r, TiXmlElement* pElement, bool avg ){
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

    if( !avg ){
      b.preload(name, file);
      } else {
      r.load(b, name, file, avg );
      }
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
#ifdef TEMPEST_OPENGL
    def += "#define opengl\n";
#endif

#ifdef TEMPEST_DIRECTX
    def += "#define directx\n";
#endif

#ifdef __ANDROID__
    def += "#define oes_render\n";
#endif

    def += "#line 0\n";
    //std::cout << def << std::endl;
    r.load(b,  name, file, def  );
    r.load(b1, name, file1, def );
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

  Model model;

  model.loadMX( vboHolder, iboHolder, "data/models/util/cube.mx" );

  models  .add("null", model);
  textures.add("null",      texHolder.load("data/textures/w.png") );
  textures.add("null/norm", texHolder.load("data/textures/norm.png") );
  texturesAvg.add( "null", Tempest::Color() );

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
    return textures.get(key, textures.get("null/norm") ); else
    return textures.get(key, textures.get("null") );
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
    return textures.get(key, textures.get("null/norm") ); else
    return textures.get(key, textures.get("null") );
  }

Tempest::Color Resource::textureAVG(const std::string &key) const {
  return texturesAvg.get(key);
  }

bool Resource::findTexture(const std::string &key) {
  return textures.contains(key);
  }

Sound &Resource::sound(const std::string &key) const {
  return *sounds.get(key);
  }

Tempest::VertexShader &Resource::vshader(const std::string &key) {
  return vs.get(key);
  }

Tempest::FragmentShader &Resource::fshader(const std::string &key){
  return fs.get(key);
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

void Resource::load( Box<Model> &m,
                     const std::string &k,
                     const std::string &f ){
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
                     const std::string & ){
  assert(0);
  }

void Resource::load( Box< Tempest::Color  > &,
                     const std::string &,
                     const std::string & ){
  assert(0);
  }

void Resource::load( Box< Tempest::VertexShader > &,
                     const std::string &,
                     const std::string & ){
  assert(0);
  }

void Resource::load( Box< Tempest::FragmentShader > &,
                     const std::string &,
                     const std::string & ){
  assert(0);
  }

void Resource::load(Box<PixmapsPool::TexturePtr> &,
                    const std::string &,
                    const std::string & ) {
  assert(0);
  }

void Resource::load(PixmapsPool::TexturePtr p, const std::string &f) {
  px.add( f, p );
  }

void Resource::load( Box< std::shared_ptr<Sound> >& sounds,
                     const std::string &k,
                     const std::string &f  ){
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

void Resource::load( Box<Tempest::Texture2d>& textures,
                     const std::string &k,
                     const std::string &f,
                     bool avg ){
  auto it = textures.loaded.find(f);

  if( it!=textures.loaded.end() ){
    textures.add(k, textures.get( it->second ) );
    } else {
    textures.add(k,  texHolder.load(f) );
    textures.loaded[f] = k;
    }

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

    texturesAvg.add(k, Tempest::Color( cl[0]/255.0,
                                    cl[1]/255.0,
                                    cl[2]/255.0,
                                    cl[3]/255.0 ) );
    }
  }

void Resource::load( Box<Tempest::VertexShader> &vs,
                     const std::string &k,
                     const std::string &f,
                     const std::string &def ) {
  std::string src = def + loadSrc(f);
  vs.add(k, vsHolder.loadFromSource(src) );
  }

void Resource::load( Box<Tempest::FragmentShader> &fs,
                     const std::string &k,
                     const std::string &f,
                     const std::string &def ) {
#ifdef __ANDROID__
  std::string src = def + "precision mediump float;\n#line 0\n" + loadSrc(f);
#else
  std::string src = def + loadSrc(f);
#endif
  fs.add(k, fsHolder.loadFromSource(src) );
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
  return Tempest::AbstractSystemAPI::loadText( f.data() );
  }
