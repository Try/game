#ifndef PAINTERGUI_H
#define PAINTERGUI_H

#include <Tempest/Painter>
#include <Tempest/Texture2d>
#include "pixmapspool.h"

namespace Tempest{
  class Device;
  }

class GUIPass;

namespace Tempest{
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

class PainterGUI: public Tempest::PainterDevice {
  public:
    PainterGUI( GUIPass & drawer,
                Resource &res, int sx, int sy, int sw, int sh );

    void line( int x0, int y0, int x1, int y1 );
    void quad( int x0, int y0, int w, int h,
               int tx, int ty, int tw, int th );

    void setTexture(const Texture &t);
    void unsetTexture();

    void setBlendMode(Tempest::BlendMode m);
    Tempest::PaintTextEngine& textEngine();

    void setColor(float r, float g, float b, float a );

    void pushState();
    void popState();
  private:
    GUIPass & drawer;
    struct TextEngine:public Tempest::PaintTextEngine{
      TextEngine( PainterGUI & p, Resource& res );

      void setFont( const Tempest::Bind::UserFont &f );
      void drawText( int x, int y, int w, int h, const std::wstring &,
                     int align = Tempest::NoAlign );
      void drawText( int x, int y, int w, int h, const std::string &,
                     int align = Tempest::NoAlign );
      PainterGUI & p;
      Resource   & res;
      const Tempest::Bind::UserFont * font;

      template< class T >
      void dText( int x, int y, int w, int h, const T &,
                  int align );
      } te;
  };

#endif // PAINTERGUI_H
