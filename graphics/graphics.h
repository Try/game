#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#include <Tempest/DirectX9>
#include <Tempest/Opengl2x>
#include <Tempest/Device>

#include <Tempest/VertexBufferHolder>
#include <Tempest/LocalVertexBufferHolder>

#include <Tempest/IndexBufferHolder>

#include <Tempest/TextureHolder>
#include <Tempest/LocalTexturesHolder>

#include <Tempest/VertexShaderHolder>
#include <Tempest/FragmentShaderHolder>

#include <Tempest/SurfaceRender>

#include <Tempest/GraphicObject>
#include <Tempest/AbstractScene>

#include "graphics/scene.h"
#include "gui/graphicssettingswidget.h"

#include <Tempest/signal>

#include "frustum.h"
#include "translate/shadermaterial.h"
#include "materialserver.h"

class ParticleSystemEngine;
class MainGui;

class Graphics {
  public:
    Graphics(void *hwnd, bool isFullScreen);

    bool render(Tempest::Surface &scene , Tempest::SpritesHolder &sp);
    bool render(Scene &scene,
                 ParticleSystemEngine &e,
                 const Tempest::Camera &camera,
                 size_t dt );

    void resizeEvent( int w, int h, bool isFullScreen );

    void load( Resource & r, MainGui & gui, int w, int h );

    void renderSubScene( const Scene &scene,
                         ParticleSystemEngine &e,
                         Tempest::Texture2d & out   );

    Tempest::signal<double> onRender;
    void setFog( const Tempest::Pixmap& p );

    void setSettings( const GraphicsSettingsWidget::Settings& s );

    enum VisibleRet{
      NotVisible = 0,
      PartialVisible,
      FullVisible
      };

    static VisibleRet isVisible( const AbstractGraphicObject& c, const Frustum& f );
    static VisibleRet isVisible( float x, float y, float z, float r, const Frustum& f );

    size_t idOfMaterial( const std::string& m );
  private:
    std::unique_ptr<Tempest::AbstractAPI> api;
    ShaderSource::Lang lang() const;

    static Tempest::Device::Options makeOpt( bool isFullScreen );
    Tempest::AbstractAPI *createAPI();
  public:
    Tempest::Device   device;

    Tempest::TextureHolder           texHolder;
    Tempest::LocalTexturesHolder     localTex;
    Tempest::VertexBufferHolder      vboHolder;
    Tempest::LocalVertexBufferHolder lvboHolder;
    Tempest::IndexBufferHolder       iboHolder;
    Tempest::LocalIndexBufferHolder  liboHolder;

    Tempest::VertexShaderHolder   vsHolder;
    Tempest::FragmentShaderHolder fsHolder;

  private:
    Resource * resource;
    Tempest::SurfaceRender r0, uiRender, hintRender;
    MainGui * widget;

    void renderImpl(Scene &scene,
                    ParticleSystemEngine &e,
                    const Tempest::Camera &camera,
                    size_t dt );

    int draw(const Frustum &frustum,
              bool deepVTest, bool shadowPass,
              const Scene::Objects & v );

    Tempest::Texture2d fillShadowMap(Tempest::Device &device, const Scene &scene , const Tempest::Size &sm);

    MaterialServer msrv;
    ShaderMaterial::UniformsContext context, cefects;

    void setupSceneConstants(const Scene &scene,
                              ShaderMaterial::UniformsContext& context );
    void setupSceneConstants(const Scene &scene,
                              ShaderMaterial::UniformsContext& context,
                              const Tempest::Matrix4x4& view,
                              const Tempest::Matrix4x4& proj );


    void initGB(GBuffer &b ,     const Tempest::Size &size);
    void initGB(ShadowBuffer& b, const Tempest::Size &size );
    Tempest::Texture2d colorBuf(int w, int h);

    Tempest::Texture2d depth(int w, int h);
    Tempest::Texture2d depth( const Tempest::Size & sz );
    Tempest::Texture2d shadowMap(int w, int h);

    Tempest::Matrix4x4 makeShadowMatrix( const Scene & scene,
                                         float * dir,
                                         float sv,
                                         bool aspect = true );
    static float smMatSize(const Scene & s, float sv = 0, float maxSv = 0);

    bool useHDR;
    Tempest::Size wndSize;
    size_t time;

    GraphicsSettingsWidget::Settings settings;

    struct SBorderVertex{
      float x,y;
      };
    Tempest::VertexDeclaration           smBorderDecl;
    Tempest::VertexBuffer<SBorderVertex> smBorder;
    Tempest::VertexShader                smBorderVs;
    Tempest::FragmentShader              smBorderFs;
  };

#endif // GRAPHICS_H
