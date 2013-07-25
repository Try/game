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

#include <Tempest/GraphicObject>
#include <Tempest/AbstractScene>

#include "graphics/guirender.h"
#include "graphics/scene.h"
#include "gui/graphicssettingswidget.h"

#include <Tempest/signal>

#include "frustum.h"
#include "translate/shadermaterial.h"

class ParticleSystemEngine;

class Graphics {
  public:
    Graphics(void *hwnd, bool isFullScreen);

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
    GuiRender gui;
    MainGui * widget;

    void renderImpl(Scene &scene,
                    ParticleSystemEngine &e,
                    const Tempest::Camera &camera,
                    size_t dt );

    std::vector<const AbstractGraphicObject*> toDraw;
    int draw( const Frustum &frustum,
              bool deepVTest,
              const Tempest::AbstractCamera & camera,
              const Scene::Objects & v );
    void completeDraw( Scene &scene);

    ShaderMaterial mat;
    ShaderMaterial::UniformsContext context;

    void setupSceneConstants( Scene& scene,
                              ShaderMaterial::UniformsContext& context );
    void setupObjectConstants(Scene& scene, const AbstractGraphicObject &obj,
                               ShaderMaterial::UniformsContext& context );

    Tempest::Size wndSize;
    size_t time;
  };

#endif // GRAPHICS_H
