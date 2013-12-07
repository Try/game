#ifndef MATERIALSERVER_H
#define MATERIALSERVER_H

#include "translate/shadermaterial.h"
#include "translate/objectcode.h"
#include "material.h"
#include "scene.h"

#include <Tempest/LocalTexturesHolder>

class Resource;
class GBuffer;
class ShadowBuffer;

class MaterialServer {
  public:
    MaterialServer( Tempest::VertexShaderHolder&   vsHolder,
                    Tempest::FragmentShaderHolder& fsHolder,
                    Tempest::LocalTexturesHolder  &localTex,
                    ShaderSource::Lang lang );

    ShaderMaterial::UniformsContext context;
    size_t idOfMaterial( const std::string& m );

    void load( Resource &res );
    void draw( const AbstractGraphicObject& obj, bool spass );
    void completeDraw( Tempest::Device &device,
                       const Scene &scene,
                       GBuffer &g,
                       Tempest::Texture2d *screen,
                       Tempest::Texture2d *depth,
                       ShaderMaterial::UniformsContext &c,
                       ShaderMaterial::UniformsContext &econtext );
    void completeDraw(Tempest::Device &device,
                       const Scene &scene,
                       ShadowBuffer &g,
                       ShaderMaterial::UniformsContext &c,
                       ShaderMaterial::UniformsContext &econtext );

    void makeWaterHMap( Tempest::Device &device,
                        const Scene &,
                        ShadowBuffer &g,
                        ShaderMaterial::UniformsContext &context,
                        ShaderMaterial::UniformsContext & );

    void setSettings(const GraphicsSettingsWidget::Settings &);
    void finishFrame();
  private:
    Tempest::VertexShaderHolder&   vsHolder;
    Tempest::FragmentShaderHolder& fsHolder;

    Tempest::LocalTexturesHolder&  localTex;
    ShaderSource::Lang lang;

    size_t shadow_main[4], main_material, water;
    size_t blt, blt_to_tex, gauss, mirron_blt;

    Tempest::Texture2d hWater[2]; bool sswater;

    struct ObjList{
        ObjList();

        struct L{
          std::vector<const AbstractGraphicObject*> obj;
          size_t vboId;
          };
        std::vector<L> obj;

        void insert( const AbstractGraphicObject* );
        size_t size() const;

        void clear();
        void collect();
      private:
        size_t m_size;
      };

    struct Mat{
      Mat( Tempest::VertexShaderHolder   &vsHolder,
           Tempest::FragmentShaderHolder &fsHolder ):m(vsHolder, fsHolder){
        //obj.reserve(1024);
        }
      ShaderMaterial m;
      ObjectCode code;

      ObjList obj;
      };
    std::vector< std::shared_ptr<Mat> > mat;
    std::vector< Mat* > gbufPass, blpass, sspass;

    struct PostProc{
      PostProc( Tempest::VertexShaderHolder &vsHolder,
                Tempest::FragmentShaderHolder &fsHolder ):m(vsHolder, fsHolder){
        }
      ShaderMaterial m;
      ObjectCode code;
      };
    std::vector< std::shared_ptr<PostProc> > efect;

    std::unordered_map<std::string, size_t> matId;

    void setupObjectConstants( const AbstractGraphicObject &obj,
                               ShaderMaterial::UniformsContext& context );

    void   pushMaterial( Resource& res, const std::string& s );
    size_t pushEfect( Resource& res, const std::string& s );

    void draw(Mat & m, Mat &obj,
              Tempest::Render &render,
              ShaderMaterial::UniformsContext &c );

    void execEfect( size_t efect,
                    Tempest::Device &dev,
                    Tempest::Texture2d *out,
                    ShaderMaterial::UniformsContext &context,
                    const Tempest::Texture2d& t );

    void setupOpt( CompileOptions & opt );

    template< class T, class ... Args >
    void drawTo( T & t,
                 Tempest::Device & device,
                 bool clr,
                 ShaderMaterial::UniformsContext& context,
                 Args&...  args );

    template< class ... Args >
    void completeDrawMMat( Tempest::Device &device,
                           const Scene &/*scene*/,
                           ShaderMaterial::UniformsContext &context,
                           ShaderMaterial::UniformsContext &/*econtext*/,
                           Args&... g  );
    Resource * res;

    GraphicsSettingsWidget::Settings settings;
  };

#endif // MATERIALSERVER_H
