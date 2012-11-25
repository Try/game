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

namespace MyGL{
  class TextureHolder;

  class VertexBufferHolder;
  class IndexBufferHolder;

  class VertexShaderHolder;
  class FragmentShaderHolder;
  }

class Resource : public AbstractXMLReader {
  public:
    Resource(
        MyGL::TextureHolder      &  texHolder,
        MyGL::VertexBufferHolder &  vboHolder,
        MyGL::IndexBufferHolder  &  iboHolder,

        MyGL::VertexShaderHolder &  vsHolder,
        MyGL::FragmentShaderHolder& fsHolder );

    const MyGL::Model<>& model( const std::string & key ) const;

    const MyGL::Texture2d &texture( const std::string & key ) const;
          MyGL::Texture2d &texture( const std::string & key );
          bool findTexture( const std::string & key );


    MyGL::VertexShader   & vshader( const std::string & key );
    MyGL::FragmentShader & fshader( const std::string & key );

    PixmapsPool::TexturePtr pixmap( const std::string & key );
    PixmapsPool::TexturePtr pixmap( const MyGL::Pixmap& pm, bool flush );

    void flushPixmaps();
    using AbstractXMLReader::load;
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

      const T& get( const std::string & s ) const {
        typename std::unordered_map< std::string, T >::const_iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;

        return data.find("null")->second;
        }

      void add( const std::string & key, const T& v ){
        data.insert( std::make_pair(key, v) );
        }
      };

    void load( Box<MyGL::Texture2d>& textures,
               const std::string &k, const std::string & f );

    void load( Box<MyGL::VertexShader>& vs,
               const std::string &k, const std::string & f,
               const std::string &def );

    void load( Box<MyGL::FragmentShader>& fs,
               const std::string &k, const std::string & f,
               const std::string &def  );

    void load( Box<MyGL::Model<> >& m,
               const std::string &k, const std::string & f );
    void load( PixmapsPool::TexturePtr p, const std::string & f );

    Box<MyGL::Model<> > models;
    Box<MyGL::Texture2d> textures;
    Box<MyGL::VertexShader>   vs;
    Box<MyGL::FragmentShader> fs;
    Box<PixmapsPool::TexturePtr> px;

    MyGL::TextureHolder      &  texHolder;
    MyGL::VertexBufferHolder &  vboHolder;
    MyGL::IndexBufferHolder  &  iboHolder;

    MyGL::VertexShaderHolder &  vsHolder;
    MyGL::FragmentShaderHolder& fsHolder;

    void readElement( TiXmlNode *node );

    struct XML;
    PixmapsPool pixmaps;
    std::string loadSrc( const std::string & f );
  };

#endif // RESOURCE_H
