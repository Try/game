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
  (void)x0;
  (void)y0;
  (void)x1;
  (void)y1;
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

void PainterGUI::setBlendMode(Tempest::BlendMode m) {
  drawer.setBlendMode(m);
  }

Tempest::PaintTextEngine &PainterGUI::textEngine() {
  return te;
  }

void PainterGUI::setColor( float r, float g, float b, float a ) {
  drawer.setColor(r, g, b, a);
  }

void PainterGUI::pushState() {
  drawer.pushState();
  }

void PainterGUI::popState() {
  drawer.popState();
  }

PainterGUI::TextEngine::TextEngine(PainterGUI &p, Resource &res)
                       :p(p), res(res), font(0) {

  }

void PainterGUI::TextEngine::setFont(const Tempest::Bind::UserFont &f) {
  font = &f;
  }

template< class T >
void PainterGUI::TextEngine::dText( int x, int y, int w, int h,
                                    const T &str,
                                    int flg ) {
  if( font==0 )
    return;

  font->fetch( res, str );

  Tempest::Rect oldScissor = p.scissor();
  p.setScissor( oldScissor.intersected( Tempest::Rect(x,y,w,h) ) );
  p.setBlendMode( Tempest::alphaBlend );

  int tx = 0, ty = 0, tw = 0, th = 0;
  for( size_t i=0; i<str.size(); ++i ){
    const Font::Leter& l = font->leter( res, str[i] );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }


  if( flg & Tempest::AlignHCenter )
    x += (w-tw)/2; else

  if( flg & Tempest::AlignRight )
    x += (w-tw);

  if( flg & Tempest::AlignVCenter )
    y += (h-th)/2; else
  if( flg & Tempest::AlignBottom )
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

void PainterGUI::TextEngine::drawText( int x, int y, int w, int h,
                                       const std::wstring &s, int align) {
  dText(x, y, w, h, s, align);
  }

void PainterGUI::TextEngine::drawText( int x, int y, int w, int h,
                                       const std::string &s, int align) {
  dText(x, y, w, h, s, align);
  }
