#ifndef PAINTERGUI_H
#define PAINTERGUI_H

#include <MyWidget/Painter>
#include <MyGL/Texture2d>
#include "pixmapspool.h"

namespace MyGL{
  class Device;
  }

class GUIPass;

namespace MyWidget{
  namespace Bind{
    struct UserTexture{
      UserTexture(){
        data.tex = 0;
        }
      PixmapsPool::TexturePtr data;
      // MyGL::Texture2d data;
      };
    }
  }

class PainterGUI: public MyWidget::PainterDevice {
  public:
    PainterGUI( GUIPass & drawer, int sx, int sy, int sw, int sh );

    void line( int x0, int y0, int x1, int y1 );
    void quad( int x0, int y0, int w, int h,
               int tx, int ty, int tw, int th );

    void setTexture(const Texture &t);
    void unsetTexture();

    void setBlendMode(MyWidget::BlendMode m);
  private:
    GUIPass & drawer;
  };

#endif // PAINTERGUI_H
