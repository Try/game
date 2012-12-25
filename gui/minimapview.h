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
    MyGL::Pixmap renderTo;
    Resource &res;

    clock_t rtime;
  };

#endif // MINIMAPVIEW_H
