#ifndef MINIMAPVIEW_H
#define MINIMAPVIEW_H

#include "textureview.h"

#include <MyGL/Texture2d>
#include <MyGL/Pixmap>

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
    MyWidget::signal<float, float, MyWidget::Event::MouseButton, Mode> mouseEvent;

  protected:
    void paintEvent(MyWidget::PaintEvent &e);

  private:
    MyGL::Texture2d terr, units, fog, hud;

    Resource &res;

    clock_t rtime, rtime2;
    bool pressed;

    void lineTo( MyGL::Pixmap &renderTo,
                 int x0, int y0, int x1, int y1 );

    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseDragEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);

    void aceptFog( MyGL::Pixmap &p, const MyGL::Pixmap &f );

    void drawUnits(MyGL::Pixmap &renderTo, World &wx);
  };

#endif // MINIMAPVIEW_H
