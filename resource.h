#ifndef RESOURCE_H
#define RESOURCE_H

#include <unordered_map>
#include <string>

#include <MyGL/Model>

#include <MyGL/Texture2d>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include "xml/abstractxmlreader.h"
#include "pixmapspool.h"

#include <memory>

namespace MyGL{
  class TextureHolder;
  class LocalTexturesHolder;

  class VertexBufferHolder;
  class IndexBufferHolder;
  class LocalVertexBufferHolder;

  class VertexShaderHolder;
  class FragmentShaderHolder;
  }

struct MVertex {
  float x,y,z;
  float u,v;
  float normal[3];
  float color[4];

  float bnormal[4];

  static MyGL::VertexDeclaration::Declarator decl();
  };

typedef MyGL::Model<MVertex> Model;

class Sound;

class Resource : public AbstractXMLReader {
  public:
    Resource(
        MyGL::TextureHolder       &  texHolder,
        MyGL::LocalTexturesHolder &  ltexHolder,

        MyGL::VertexBufferHolder &  vboHolder,
        MyGL::LocalVertexBufferHolder &lvboHolder,
        MyGL::IndexBufferHolder  &  iboHolder,

        MyGL::VertexShaderHolder &  vsHolder,
        MyGL::FragmentShaderHolder& fsHolder );

    const Model& model( const std::string & key ) const;
    const Model::Raw& rawModel( const std::string & key ) const;
    Model model(const MyGL::Model<MVertex>::Raw& r) const;

    const MyGL::Texture2d &texture( const std::string & key ) const;
          MyGL::Texture2d &texture( const std::string & key );
          bool findTexture( const std::string & key );

    Sound &sound( const std::string & key ) const;


    MyGL::VertexShader   & vshader( const std::string & key );
    MyGL::FragmentShader & fshader( const std::string & key );

    PixmapsPool::TexturePtr pixmap( const std::string & key );
    PixmapsPool::TexturePtr pixmap( const MyGL::Pixmap& pm, bool flush );

    void flushPixmaps();
    using AbstractXMLReader::load;

    MyGL::TextureHolder       &  texHolder;
    MyGL::LocalTexturesHolder &  ltexHolder;
private:
    template< class T >
    struct Box{
      std::unordered_map< std::string, T > data;

      bool contains( const std::string & s ){
        typename std::unordered_map< std::string, T >::iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return true;

        return false;
        }

      T& get( const std::string & s ){
        typename std::unordered_map< std::string, T >::iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;

        return data["null"];
        }

      T& get( const std::string & s, T& def ){
        typename std::unordered_map< std::string, T >::iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;

        return def;
        }

      const T& get( const std::string & s ) const {
        typename std::unordered_map< std::string, T >::const_iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;

        return data.find("null")->second;
        }

      const T& get( const std::string & s, const T& def ) const {
        typename std::unordered_map< std::string, T >::const_iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;

        return def;
        }

      void add( const std::string & key, const T& v ){
        data.insert( std::make_pair(key, v) );
        }

      std::unordered_map< std::string, std::string > loaded;
      };

    void load( Box<MyGL::Texture2d>& textures,
               const std::string &k, const std::string & f );

    void load( Box< std::shared_ptr<Sound> >& sounds,
               const std::string &k, const std::string & f );

    void load( Box<MyGL::VertexShader>& vs,
               const std::string &k, const std::string & f,
               const std::string &def );

    void load( Box<MyGL::FragmentShader>& fs,
               const std::string &k, const std::string & f,
               const std::string &def  );

    void load( Box<Model >& m,
               const std::string &k, const std::string & f );
    void load( PixmapsPool::TexturePtr p, const std::string & f );

    Box<Model > models;
    mutable Box< std::shared_ptr<Model::Raw> > rawModels;
    Box<MyGL::Texture2d> textures;
    Box<MyGL::VertexShader>   vs;
    Box<MyGL::FragmentShader> fs;
    Box<PixmapsPool::TexturePtr> px;

    Box< std::shared_ptr<Sound> > sounds;

    MyGL::VertexBufferHolder      & vboHolder;
    MyGL::LocalVertexBufferHolder & lvboHolder;
    MyGL::IndexBufferHolder       & iboHolder;

    MyGL::VertexShaderHolder &  vsHolder;
    MyGL::FragmentShaderHolder& fsHolder;

    void readElement( TiXmlNode *node );

    struct XML;
    PixmapsPool pixmaps;
    std::string loadSrc( const std::string & f );

    static void computeBiNormal( MVertex & a,
                                 MVertex & b,
                                 MVertex & c );
  };

#endif // RESOURCE_H
