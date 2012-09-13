#include "paintergui.h"

#include "graphics/guipass.h"

PainterGUI::PainterGUI( GUIPass & d, int sx, int sy, int sw, int sh ):drawer(d) {
  unsetTexture();
  drawer.clearBuffers();

  setScissor(sx, sy, sw, sh);
  }

void PainterGUI::line( int x0, int y0, int x1, int y1 ) {
  //painter->setPen( makeColor(c0) );
  //painter->drawLine( x0, y0, x1, y1 );
  }

void PainterGUI::quad(int x0, int y0, int w, int h,
                      int tx, int ty, int tw, int th ) {
  drawer.rect( x0, y0, x0+w, y0+h, tx, ty, tw, th );
  }

void PainterGUI::setTexture(const Texture &t) {
  drawer.setTexture( t.data );
  }

void PainterGUI::unsetTexture() {
  drawer.unsetTexture();
  }

void PainterGUI::setBlendMode(MyWidget::BlendMode m) {
  drawer.setBlendMode(m);
  }

