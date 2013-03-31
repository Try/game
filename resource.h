#ifndef RESOURCE_H
#define RESOURCE_H

#include "stlconf.h"

#include <unordered_map>
#include <string>

#include <Tempest/Model>

#include <Tempest/Texture2d>
#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

#include "xml/abstractxmlreader.h"
#include "pixmapspool.h"
#include "model_mx.h"

#include <memory>

namespace Tempest{
  class TextureHolder;
  class LocalTexturesHolder;

  class VertexBufferHolder;
  class IndexBufferHolder;

  class LocalVertexBufferHolder;

  class VertexShaderHolder;
  class FragmentShaderHolder;
  }

class Sound;

class Resource : public AbstractXMLReader {
  public:
    Resource(
        Tempest::TextureHolder       &  texHolder,
        Tempest::LocalTexturesHolder &  ltexHolder,

        Tempest::VertexBufferHolder &  vboHolder,
        Tempest::LocalVertexBufferHolder &lvboHolder,
        Tempest::IndexBufferHolder  &  iboHolder,

        Tempest::VertexShaderHolder &  vsHolder,
        Tempest::FragmentShaderHolder& fsHolder );

    const Model& model( const std::string & key ) const;
    const Model::Raw& rawModel( const std::string & key ) const;
    Model model(const Tempest::Model<MVertex>::Raw& r) const;

    const Tempest::Texture2d &texture( const std::string & key ) const;
          Tempest::Texture2d &texture( const std::string & key );
          Tempest::Color textureAVG( const std::string & key ) const;
          bool findTexture( const std::string & key );

    Sound &sound( const std::string & key ) const;


    Tempest::VertexShader   & vshader( const std::string & key );
    Tempest::FragmentShader & fshader( const std::string & key );

    PixmapsPool::TexturePtr pixmap( const std::string & key );
    PixmapsPool::TexturePtr pixmap( const Tempest::Pixmap& pm, bool flush );

    void flushPixmaps();
    using AbstractXMLReader::load;

    Tempest::TextureHolder       &  texHolder;
    Tempest::LocalTexturesHolder &  ltexHolder;
private:
    template< class T >
    struct Box{
      Box():owner(0){}

      Resource * owner;
      std::unordered_map< std::string, T > data;
      std::unordered_map< std::string, std::string > toLoad;

      bool contains( const std::string & s ){
        if( data.find(s)!=data.end() )
          return true;

        if( toLoad.find(s)!=toLoad.end() )
          return true;

        return false;
        }

      T& get( const std::string & s ){
        {
        typename std::unordered_map< std::string, std::string >::const_iterator i;
        i = toLoad.find(s);

        if( i!=toLoad.end() ){
          owner->load(*this, s, i->second);
          toLoad.erase(i);
          }
        }

        {
        typename std::unordered_map< std::string, T >::iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;
        }

        return data["null"];
        }

      T& get( const std::string & s, T& def ){
        {
        typename std::unordered_map< std::string, std::string >::const_iterator i;
        i = toLoad.find(s);

        if( i!=toLoad.end() ){
          owner->load(*this, s, i->second);
          toLoad.erase(i);
          }
        }

        {
        typename std::unordered_map< std::string, T >::iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;
        }

        return def;
        }

      const T& get( const std::string & s ) const {
        {
        typename std::unordered_map< std::string, std::string >::const_iterator i;
        i = toLoad.find(s);

        if( i!=toLoad.end() ){
          owner->load(*this, s, i->second);
          toLoad.erase(i);
          }
        }

        {
        typename std::unordered_map< std::string, T >::const_iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;
        }

        return data.find("null")->second;
        }

      const T& get( const std::string & s, const T& def ) const {
        {
        typename std::unordered_map< std::string, std::string >::const_iterator i;
        i = toLoad.find(s);

        if( i!=toLoad.end() ){
          owner->load(*this, s, i->second);
          toLoad.erase(i);
          }
        }

        {
        typename std::unordered_map< std::string, T >::const_iterator i;
        i = data.find(s);

        if( i!=data.end() )
          return i->second;
        }

        return def;
        }

      void preload( const std::string & key, const std::string& v ){
        toLoad.insert( std::make_pair(key, v) );
        }

      void add( const std::string & key, const T& v ){
        data.insert( std::make_pair(key, v) );
        }

      std::unordered_map< std::string, std::string > loaded;
      };

    void load( Box<Tempest::Texture2d>& textures,
               const std::string &k, const std::string & f, bool avg = false );

    void load( Box< std::shared_ptr<Sound> >& sounds,
               const std::string &k, const std::string & f );

    void load( Box<Tempest::VertexShader>& vs,
               const std::string &k, const std::string & f,
               const std::string &def );

    void load( Box<Tempest::FragmentShader>& fs,
               const std::string &k, const std::string & f,
               const std::string &def  );

    void load( Box<Model >& m,
               const std::string &k, const std::string & f );

    void load( Box< std::shared_ptr<Model::Raw> >& m,
               const std::string &k, const std::string & f );

    void load( Box<Tempest::Color>& m,
               const std::string &k, const std::string & f );

    void load( Box<Tempest::VertexShader>& vs,
               const std::string &k, const std::string & f );

    void load( Box<Tempest::FragmentShader>& fs,
               const std::string &k, const std::string & f );

    void load( Box<PixmapsPool::TexturePtr>& fs,
               const std::string &k, const std::string & f );

    void load( PixmapsPool::TexturePtr p, const std::string & f );

    mutable Box< Model > models;
    mutable Box< std::shared_ptr<Model::Raw> > rawModels;
    mutable Box<Tempest::Texture2d> textures;
    mutable Box<Tempest::Color>     texturesAvg;
    Box<Tempest::VertexShader>   vs;
    Box<Tempest::FragmentShader> fs;
    Box<PixmapsPool::TexturePtr> px;

    mutable Box< std::shared_ptr<Sound> > sounds;

    Tempest::VertexBufferHolder      & vboHolder;
    Tempest::LocalVertexBufferHolder & lvboHolder;
    Tempest::IndexBufferHolder       & iboHolder;

    Tempest::VertexShaderHolder &  vsHolder;
    Tempest::FragmentShaderHolder& fsHolder;

    void readElement( TiXmlNode *node );

    struct XML;
    PixmapsPool pixmaps;
    std::string loadSrc( const std::string & f );
  };

#endif // RESOURCE_H
