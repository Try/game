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
  private:
    MyGL::Pixmap renderTo, cashed;
    Resource &res;

    clock_t rtime;

    void lineTo( MyGL::Pixmap &renderTo,
                 int x0, int y0, int x1, int y1 );
  };

#endif // MINIMAPVIEW_H
