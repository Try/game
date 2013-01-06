#include "resource.h"

#include <MyGL/VertexShaderHolder>
#include <MyGL/LocalVertexBufferHolder>

#include <tinyxml.h>
#include <iostream>
#include <fstream>
#include <cmath>

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



MyGL::VertexDeclaration::Declarator MVertex::decl() {
  MyGL::VertexDeclaration::Declarator d;
  d   .add( MyGL::Decl::float3, MyGL::Usage::Position )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord )
      .add( MyGL::Decl::float3, MyGL::Usage::Normal   )
      .add( MyGL::Decl::float4, MyGL::Usage::Color    )
      .add( MyGL::Decl::float4, MyGL::Usage::BiNormal );

  return d;
  }

void Resource::computeBiNormal( MVertex &va, MVertex &vb, MVertex &vc ) {
  float a[3] = { va.x-vc.x, va.y-vc.y, va.z-vc.z };
  float b[3] = { vb.x-vc.x, vb.y-vc.y, vb.z-vc.z };

  float t1[2] = { va.u-vc.u, va.v-vc.v };
  float t2[2] = { vb.u-vc.u, vb.v-vc.v };

  if( fabs(t2[1]) > 0.00001 ){
    float k = t1[1]/t2[1];
    float m = ( t1[0]-t2[0]*k );

    float u[4] = { a[0]-b[0]*k, a[1]-b[1]*k, a[2]-b[2]*k, 0 };
    for( int i=0; i<3; ++i )
      u[i] /= m;
    /*
    float l = 0;
    for( int i=0; i<3; ++i )
      l += u[i]*u[i];//u[i]*=m;
    l = sqrt(l);
    for( int i=0; i<3; ++i )
      u[i]/= l;
      */

    for( int i=0; i<4; ++i ){
      va.bnormal[i] = -u[i];
      vb.bnormal[i] = -u[i];
      vc.bnormal[i] = -u[i];
      }
    //float v[3] = {};
    } else {
    float u[4] = { b[0]/t2[0], b[1]/t2[0], b[2]/t2[0], 0 };

    for( int i=0; i<4; ++i ){
      va.bnormal[i] = -u[i];
      vb.bnormal[i] = -u[i];
      vc.bnormal[i] = -u[i];
      }
    }
  }

Resource::Resource( MyGL::TextureHolder       & tx,
                    MyGL::LocalTexturesHolder & ltx,

                    MyGL::VertexBufferHolder &  vh,
                    MyGL::LocalVertexBufferHolder &lvh,
                    MyGL::IndexBufferHolder  &  ih,

                    MyGL::VertexShaderHolder &  vsh,
                    MyGL::FragmentShaderHolder& fsh )
  : texHolder(tx),
    ltexHolder(ltx),
    vboHolder(vh),
    lvboHolder(lvh),
    iboHolder(ih),
    vsHolder(vsh),
    fsHolder(fsh),
    pixmaps( texHolder ){
  Model model;
  MyGL::Model<>::Raw raw = MyGL::Model<>::loadRawData("./data/models/model.mx");

  Model::Raw rawN;
  rawN.vertex.resize( raw.vertex.size() );
  for( size_t i=0; i<rawN.vertex.size(); ++i ){
    MVertex             &v = rawN.vertex[i];
    MyGL::DefaultVertex &d = raw.vertex[i];

    memcpy( &v, &d, sizeof(d) );
    std::fill( v.color, v.color+4, 1) ;
    }

  for( size_t i=0; i<rawN.vertex.size(); i+=3 ){
    computeBiNormal( rawN.vertex[i  ],
                     rawN.vertex[i+1],
                     rawN.vertex[i+2] );
    }

  model.load( vboHolder, iboHolder, rawN, MVertex::decl() );
  //model.load( vboHolder, iboHolder, "./data/models/model.mx" );

  models  .add("null", model);
  textures.add("null",      texHolder.load("./data/textures/w.png") );
  textures.add("null/norm", texHolder.load("./data/textures/norm.png") );

  vs.add("null", vsHolder.load("./data/sh/main_material.vert") );
  fs.add("null", fsHolder.load("./data/sh/main_material.frag") );
  }

const Model& Resource::model(const std::string &key) const {
  return models.get(key);
  }

Model Resource::model( const MyGL::Model<MVertex>::Raw &r ) const {
  Model m;

  m.load( lvboHolder, iboHolder, r, MVertex::decl() );

  return m;
  }

const MyGL::Texture2d &Resource::texture(const std::string &key) const {
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

MyGL::Texture2d &Resource::texture(const std::string &key) {
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

void Resource::load( Box<Model> &m,
                     const std::string &k,
                     const std::string &f ){
  auto it = m.loaded.find(f);

  if( it!=m.loaded.end() ){
    m.add(k, m.get( it->second ) );
    } else {
    Model model;

    MyGL::Model<>::Raw raw = MyGL::Model<>::loadRawData( f );

    Model::Raw rawN;
    rawN.vertex.resize( raw.vertex.size() );
    for( size_t i=0; i<rawN.vertex.size(); ++i ){
      MVertex             &v = rawN.vertex[i];
      MyGL::DefaultVertex &d = raw.vertex[i];

      memcpy( &v, &d, sizeof(d) );
      std::fill( v.color, v.color+4, 1) ;
      }

    for( size_t i=0; i<rawN.vertex.size(); i+=3 ){
      computeBiNormal( rawN.vertex[i  ],
                       rawN.vertex[i+1],
                       rawN.vertex[i+2] );
      }

    model.load( vboHolder, iboHolder, rawN, MVertex::decl() );
    /*
    Model model;
    model.load( vboHolder, iboHolder, f );*/

    m.add(k, model );
    m.loaded[f] = k;
    }
  }

void Resource::load(PixmapsPool::TexturePtr p, const std::string &f) {
  px.add( f, p );
  }

void Resource::load( Box<MyGL::Texture2d>& textures,
                     const std::string &k,
                     const std::string &f  ){
  auto it = textures.loaded.find(f);

  if( it!=textures.loaded.end() ){
    textures.add(k, textures.get( it->second ) );
    } else {
    textures.add(k,  texHolder.load(f) );
    textures.loaded[f] = k;
    }
  }

void Resource::load( Box<MyGL::VertexShader> &vs,
                     const std::string &k,
                     const std::string &f,
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
