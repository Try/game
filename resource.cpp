#include "resource.h"

#include <MyGL/VertexShaderHolder>

#include <tinyxml.h>
#include <iostream>
#include <fstream>

struct Resource::XML{
  template< class Box >
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

    r.load(b, name, file );
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

    r.load( pm.add( MyGL::Pixmap(file) ), name );
    }
  };

Resource::Resource( MyGL::TextureHolder      &  tx,
                    MyGL::VertexBufferHolder &  vh,
                    MyGL::IndexBufferHolder  &  ih,

                    MyGL::VertexShaderHolder &  vsh,
                    MyGL::FragmentShaderHolder& fsh )
  : texHolder(tx),
    vboHolder(vh),
    iboHolder(ih),
    vsHolder(vsh),
    fsHolder(fsh),
    pixmaps( texHolder ){
  Model model;
  MyGL::VertexDeclaration::Declarator decl;
  decl.add( MyGL::Decl::float3, MyGL::Usage::Position )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord )
      .add( MyGL::Decl::float3, MyGL::Usage::Normal   )
      .add( MyGL::Decl::float4, MyGL::Usage::Color    );

  MyGL::Model<>::Raw raw = MyGL::Model<>::loadRawData("./data/models/model.mx");

  Model::Raw rawN;
  rawN.vertex.resize( raw.vertex.size() );
  for( size_t i=0; i<rawN.vertex.size(); ++i ){
    MVertex             &v = rawN.vertex[i];
    MyGL::DefaultVertex &d = raw.vertex[i];

    memcpy( &v, &d, sizeof(d) );
    std::fill( v.color, v.color+4, 1) ;
    }
  model.load( vboHolder, iboHolder, rawN, decl );
  //model.load( vboHolder, iboHolder, "./data/models/model.mx" );

  models  .add("null", model);
  textures.add("null", texHolder.load("./data/textures/w.png") );

  vs.add("null", vsHolder.load("./data/sh/main_material.vert") );
  fs.add("null", fsHolder.load("./data/sh/main_material.frag") );
  }

const Model& Resource::model(const std::string &key) const {
  return models.get(key);
  }

Model Resource::model( const MyGL::Model<MVertex>::Raw &r ) const {
  Model m;

  MyGL::VertexDeclaration::Declarator decl;
  decl.add( MyGL::Decl::float3, MyGL::Usage::Position )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord )
      .add( MyGL::Decl::float3, MyGL::Usage::Normal   )
      .add( MyGL::Decl::float4, MyGL::Usage::Color    );

  m.load( vboHolder, iboHolder, r, decl );

  return m;
  }

const MyGL::Texture2d &Resource::texture(const std::string &key) const {
  return textures.get(key);
  }

MyGL::Texture2d &Resource::texture(const std::string &key) {
  return textures.get(key);
  }

bool Resource::findTexture(const std::string &key) {
  return textures.contains(key);
  }

MyGL::VertexShader &Resource::vshader(const std::string &key) {
  return vs.get(key);
  }

MyGL::FragmentShader &Resource::fshader(const std::string &key){
  return fs.get(key);
  }

PixmapsPool::TexturePtr Resource::pixmap(const std::string &key) {
  //pixmaps.flush();
  return px.get(key);
  }

PixmapsPool::TexturePtr Resource::pixmap( const MyGL::Pixmap &px,
                                          bool flush ) {
  PixmapsPool::TexturePtr tmp = pixmaps.add(px);

  if( flush )
    pixmaps.flush();

  return tmp;
  }

void Resource::flushPixmaps() {
  pixmaps.flush();
  }

void Resource::load(Box<Model> &m,
                    const std::string &k,
                    const std::string &f){
  Model model;
  MyGL::VertexDeclaration::Declarator decl;
  decl.add( MyGL::Decl::float3, MyGL::Usage::Position )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord )
      .add( MyGL::Decl::float3, MyGL::Usage::Normal   )
      .add( MyGL::Decl::float4, MyGL::Usage::Color    );

  MyGL::Model<>::Raw raw = MyGL::Model<>::loadRawData( f );

  Model::Raw rawN;
  rawN.vertex.resize( raw.vertex.size() );
  for( size_t i=0; i<rawN.vertex.size(); ++i ){
    MVertex             &v = rawN.vertex[i];
    MyGL::DefaultVertex &d = raw.vertex[i];

    memcpy( &v, &d, sizeof(d) );
    std::fill( v.color, v.color+4, 1) ;
    }
  model.load( vboHolder, iboHolder, rawN, decl );
  /*
  Model model;
  model.load( vboHolder, iboHolder, f );*/

  m.add(k, model );
  }

void Resource::load(PixmapsPool::TexturePtr p, const std::string &f) {
  px.add( f, p );
  }

void Resource::load(Box<MyGL::Texture2d>& textures,
                    const std::string &k,
                    const std::string &f){
  textures.add(k,  texHolder.load(f) );
  }

void Resource::load( Box<MyGL::VertexShader> &vs,
                     const std::string &k, const std::string &f,
                     const std::string &def ) {
  std::string src = def + loadSrc(f);
  vs.add(k, vsHolder.loadFromSource(src) );
  }

void Resource::load( Box<MyGL::FragmentShader> &fs,
                     const std::string &k, const std::string &f,
                     const std::string &def ) {
  std::string src = def + loadSrc(f);
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
      XML::loadFile( textures, *this, node->ToElement() );
      } else

    if( type=="shader"){
      XML::loadFile( vs, fs, *this, node->ToElement() );
      } else

    if( type=="pixmap"){
      XML::loadFile( pixmaps, *this, node->ToElement() );
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
  int length;
  std::string buffer;

  std::ifstream is;
  is.open( f.data(), std::ios::binary );

  is.seekg (0, std::ios::end);
  length = is.tellg();
  is.seekg (0, std::ios::beg);

  buffer.resize( length );

  is.read ( &buffer[0], length );
  is.close();

  return buffer;
  }

