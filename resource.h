#ifndef RESOURCE_H
#define RESOURCE_H

#include <unordered_map>
#include <string>

#include <Tempest/Model>

#include <Tempest/Texture2d>
#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>
#include <Tempest/ResourceContext>

#include <Tempest/SpritesHolder>
#include <Tempest/Sprite>

#include "pixmapspool.h"
#include "model_mx.h"

#include "gui/graphicssettingswidget.h"
#include "graphics/translate/objectcode.h"

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

class Resource : public Tempest::ResourceContext {
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
    const ObjectCode& material( const std::string& key ) const;
    const std::vector<std::string>& unitsMaterials() const;

    Tempest::VertexShader   & vshader( const std::string & key );
    Tempest::FragmentShader & fshader( const std::string & key );

    Tempest::Sprite pixmap( const std::string & key );
    Tempest::Sprite pixmap( const Tempest::Pixmap& pm, bool flush );
    Tempest::SpritesHolder& sprites();

    void flushPixmaps();
    //using AbstractXMLReader::load;
    void load( const std::string & s );

    Tempest::TextureHolder       &  texHolder;
    Tempest::LocalTexturesHolder &  ltexHolder;

    struct MaterialOpt{
      bool zwriting;
      };

    void setupSettings( const GraphicsSettingsWidget::Settings& settings );
  private:
    GraphicsSettingsWidget::Settings settings;

    template< class T >
    struct Box{
      Box():owner(0){}

      Resource * owner;
      std::unordered_map< std::string, T > data;

      struct ToLoadPromision{
        std::string fname;
        bool        avg;
        };
      mutable std::unordered_map< std::string, ToLoadPromision > toLoad;

      bool contains( const std::string & s ){
        if( data.find(s)!=data.end() )
          return true;

        if( toLoad.find(s)!=toLoad.end() )
          return true;

        return false;
        }

      T& get( const std::string & s ){
        {
        typename std::unordered_map< std::string, ToLoadPromision >::const_iterator i;
        i = toLoad.find(s);

        if( i!=toLoad.end() ){
          owner->load( *this, s, i->second.fname, i->second.avg );
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
        typename std::unordered_map< std::string, ToLoadPromision >::const_iterator i;
        i = toLoad.find(s);

        if( i!=toLoad.end() ){
          owner->load(*this, s, i->second.fname, i->second.avg );
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
        typename std::unordered_map< std::string, ToLoadPromision >::const_iterator i;
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
        ToLoadPromision p;
        p.fname = v;
        p.avg   = false;

        toLoad.insert( std::make_pair(key, p) );
        }

      void preload( const std::string & key, const std::string& v, bool avg ){
        ToLoadPromision p;
        p.fname = v;
        p.avg   = avg;

        toLoad.insert( std::make_pair(key, p) );
        }

      void add( const std::string & key, const T& v ){
        data.insert( std::make_pair(key, v) );
        }

      std::unordered_map< std::string, std::string > loaded;
      };

    template< class Sh >
    struct Shader{
      Sh sh;
      std::string settings;
      std::string def, src;
      };
    typedef Shader<Tempest::VertexShader>   VShader;
    typedef Shader<Tempest::FragmentShader> FShader;

    struct Texture{
      bool avg; Tempest::Color color;
      Tempest::Texture2d       data;
      };

    //std::unordered_map<MaterialOpt, std::string> matOpt;

    void load( Box<Texture>& textures,
               const std::string &k, const std::string & f, bool );

    void load(Box< std::shared_ptr<Sound> >& sounds,
               const std::string &k, const std::string & f , bool);

    void load(Box< std::shared_ptr<ObjectCode> >& sounds,
               const std::string &k, const std::string & f , bool);

    void load( Box<VShader>& vs,
               const std::string &k, const std::string & f,
               const std::string &def, bool );

    void load( Box<FShader>& fs,
               const std::string &k, const std::string & f,
               const std::string &def, bool  );

    void load( Box<Model >& m,
               const std::string &k, const std::string & f, bool );

    void load(Box< std::shared_ptr<Model::Raw> >& m,
               const std::string &k, const std::string & f , bool);

    void load( const Box< std::shared_ptr<ObjectCode> >& m,
               const std::string &k,
               const Box< std::shared_ptr<ObjectCode> >::ToLoadPromision &);

    //void load( Box<Tempest::Color>& m,
    //           const std::string &k, const std::string & f );

    void load( Box< VShader >& vs,
               const std::string &k, const std::string & f , bool);

    void load( Box< FShader >& fs,
               const std::string &k, const std::string & f, bool );

    void load( Box<Tempest::Sprite>& fs,
               const std::string &k, const std::string & f , bool);

    void load(const Tempest::Sprite &p, const std::string & f );

    mutable Box< Model > models;
    mutable Box< std::shared_ptr<Model::Raw> > rawModels;
    mutable Box< Texture > textures;
    //mutable Box<Tempest::Color>     texturesAvg;
    Box< VShader > vs;
    Box< FShader > fs;
    Box< Tempest::Sprite > px;

    mutable Box< std::shared_ptr<Sound> > sounds;
    Box< std::shared_ptr<ObjectCode> > materials;
    std::vector<std::string> umaterials;

    Tempest::VertexBufferHolder      & vboHolder;
    Tempest::LocalVertexBufferHolder & lvboHolder;
    Tempest::IndexBufferHolder       & iboHolder;

    Tempest::VertexShaderHolder &  vsHolder;
    Tempest::FragmentShaderHolder& fsHolder;

    Tempest::SpritesHolder pixmaps;
    std::string settingsStr;
    std::string loadSrc( const std::string & f );
  };

#endif // RESOURCE_H
