#ifndef MINIMAPVIEW_H
#define MINIMAPVIEW_H

#include "textureview.h"

#include <Tempest/Texture2d>
#include <Tempest/Pixmap>

#include "game/world.h"

class World;

class MiniMapView : public TextureView {
  public:
    MiniMapView( Resource & res );
    ~MiniMapView();

    void setup(World *w );
    void render();

    enum Mode{
      Up,
      Drag,
      Down
      };
    Tempest::signal<float, float, Tempest::Event::MouseButton, Mode> mouseEvent;

  protected:
    void paintEvent(Tempest::PaintEvent &e);

  private:
    Tempest::Texture2d terr, units, fog, hud;

    Resource &res;

    size_t rtime, rtime2, tcount;
    World::CameraViewBounds camBounds;
    bool pressed;

    void lineTo(Tempest::Pixmap &renderTo,
                 int x0, int y0, int x1, int y1 , Tempest::Pixmap::Pixel px);

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);

    void aceptFog( Tempest::Pixmap &p, const Tempest::Pixmap &f );

    void drawUnits(Tempest::Pixmap &renderTo, World &wx);

    Tempest::Pixmap hudPx;

    World * world;
    bool    needToUpdateTerrain;

    void onTerrainCanged();
  };

#endif // MINIMAPVIEW_H
