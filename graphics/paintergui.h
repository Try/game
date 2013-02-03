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
      };
    }
  }

class Resource;

class PainterGUI: public MyWidget::PainterDevice {
  public:
    PainterGUI( GUIPass & drawer,
                Resource &res, int sx, int sy, int sw, int sh );

    void line( int x0, int y0, int x1, int y1 );
    void quad( int x0, int y0, int w, int h,
               int tx, int ty, int tw, int th );

    void setTexture(const Texture &t);
    void unsetTexture();

    void setBlendMode(MyWidget::BlendMode m);
    MyWidget::PaintTextEngine& textEngine();

    void setColor(float r, float g, float b, float a );

    void pushState();
    void popState();
  private:
    GUIPass & drawer;
    struct TextEngine:public MyWidget::PaintTextEngine{
      TextEngine( PainterGUI & p, Resource& res );

      void setFont( const MyWidget::Bind::UserFont &f );
      void drawText( int x, int y, int w, int h, const std::wstring &,
                     int align = MyWidget::NoAlign );
      PainterGUI & p;
      Resource   & res;
      const MyWidget::Bind::UserFont * font;
      } te;
  };

#endif // PAINTERGUI_H
