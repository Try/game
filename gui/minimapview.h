#ifndef MINIMAPVIEW_H
#define MINIMAPVIEW_H

#include "textureview.h"

#include <Tempest/Texture2d>
#include <Tempest/Pixmap>

#include <ctime>

class World;

class MiniMapView : public TextureView {
  public:
    MiniMapView( Resource & res );

    void render( World& w );

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

    clock_t rtime, rtime2;
    bool pressed;

    void lineTo( Tempest::Pixmap &renderTo,
                 int x0, int y0, int x1, int y1 );

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);

    void aceptFog( Tempest::Pixmap &p, const Tempest::Pixmap &f );

    void drawUnits(Tempest::Pixmap &renderTo, World &wx);
  };

#endif // MINIMAPVIEW_H
