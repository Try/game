#include "paintergui.h"

#include "graphics/guipass.h"
#include "gui/font.h"

#include <cstddef>

PainterGUI::PainterGUI(GUIPass & d, Resource &res, int sx, int sy, int sw, int sh )
           :drawer(d), te(*this, res) {
  unsetTexture();
  //drawer.clearBuffers();

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

MyWidget::PaintTextEngine &PainterGUI::textEngine() {
  return te;
  }

PainterGUI::TextEngine::TextEngine(PainterGUI &p, Resource &res)
                       :p(p), res(res), font(0) {

  }

void PainterGUI::TextEngine::setFont(const MyWidget::Bind::UserFont &f) {
  font = &f;
  }

void PainterGUI::TextEngine::drawText( int x, int y, int w, int h,
                                       const std::wstring &str,
                                       int flg ) {
  if( font==0 )
    return;

  font->fetch( res, str );

  MyWidget::Rect oldScissor = p.scissor();
  p.setScissor( oldScissor.intersected( MyWidget::Rect(x,y,w,h) ) );
  p.setBlendMode( MyWidget::alphaBlend );

  int tx = 0, ty = 0, tw = 0, th = 0;
  for( size_t i=0; i<str.size(); ++i ){
    const Font::Leter& l = font->leter( res, str[i] );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }


  if( flg & MyWidget::AlignHCenter )
    x += (w-tw)/2; else

  if( flg & MyWidget::AlignRight )
    x += (w-tw);

  if( flg & MyWidget::AlignVCenter )
    y += (h-th)/2; else
  if( flg & MyWidget::AlignBottom )
    y += (h-th);


  for( size_t i=0; i<str.size(); ++i ){
    const Font::Leter& l = font->leter( res, str[i] );
    p.setTexture( l.surf );
    p.drawRect( x+l.dpos.x, y+l.dpos.y, l.size.w, l.size.h,
                0,0 );
    x+= l.advance.x;
    y+= l.advance.y;
    }

  p.setScissor(oldScissor);
  }
