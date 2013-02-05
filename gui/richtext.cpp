#include "richtext.h"

#include <MyWidget/Painter>
#include "font.h"

RichText::RichText( Resource & res ):res(res) {
  txt = L"";
  }

void RichText::renderText( int x, int y,
                           Resource & res,
                           MyWidget::Painter &p,
                           const std::wstring &txt) {
  MyWidget::Size r;
  rText(x, y, res, &p, txt, r);
  }

MyWidget::Size RichText::bounds( Resource & res,
                                 const std::wstring &txt ) {
  MyWidget::Size r;
  rText(0, 0, res, 0, txt, r);

  return r;
  }

void RichText::setText(const std::wstring &text) {
  txt = text;
  update();
  }

void RichText::rText( int dx, int dy,
                      Resource &res,
                      MyWidget::Painter *p,
                      const std::wstring &txt,
                      MyWidget::Size & rect ) {
  Font font(15);

  if( p ){
    p->setFont(font);
    p->setBlendMode( MyWidget::alphaBlend );
    }

  int x = 0, y = 0, w = 0, h = 0;
  int ddy = 0;

  for( size_t i=0; i<txt.size(); ++i ){
    size_t pr = 0;

    if( txt[i]=='\n' ){
      x = 0;
      y += ddy;
      Font::Leter l = font.leter(res, txt[i]);

      ddy = l.dpos.y + l.size.h;
      }
    else
    if( (pr = prefix( txt, i, L"<b>" )) != size_t(-1) ){
      font.setBold(1);
      i = pr;
      }
    else
    if( (pr = prefix( txt, i, L"</b>" )) != size_t(-1) ){
      font.setBold(0);
      i = pr;
      }
    else
    if( (pr = prefix( txt, i, L"<i>" )) != size_t(-1) ){
      font.setItalic(1);
      i = pr;
      }
    else
    if( (pr = prefix( txt, i, L"</i>" )) != size_t(-1) ){
      font.setItalic(0);
      i = pr;
      }
    else
    if( (pr = prefix( txt, i, L"<size=" )) != size_t(-1) ){
      ++pr;
      while( pr<txt.size() && isSpace(txt[pr]) )
        ++pr;

      int sz = 0;
      while( pr<txt.size() &&
             '0' <= txt[pr] &&
             '9' >= txt[pr] ){
        sz = sz*10 + txt[pr]-'0';
        ++pr;
        }

      font.setSize( sz );
      i = pr;
      size_t pr2 = 0;
      if( (pr2 = prefix( txt, i, L"/>" )) != size_t(-1) )
        i = pr2;
      }
    else
    if( (pr = prefix( txt, i, L"<color=" )) != size_t(-1) ){
      ++pr;
      while( pr<txt.size() && isSpace(txt[pr]) )
        ++pr;

      std::string color;
      while( pr<txt.size() &&
             (('0' <= txt[pr] &&
               '9' >= txt[pr]) ||
             ( 'A' <= txt[pr] &&
               'F' >= txt[pr] ) ||
             ( 'a' <= txt[pr] &&
               'f' >= txt[pr] ) ) ){
        color.push_back( txt[pr] );
        ++pr;
        }

      if( p && color.size()==6 ){
        color.push_back('f');
        color.push_back('f');
        }

      if( p && color.size()==8 ){
        float r = cToInt(color[0], color[1])/255.0;
        float g = cToInt(color[2], color[3])/255.0;
        float b = cToInt(color[4], color[5])/255.0;
        float a = cToInt(color[6], color[7])/255.0;


        PainterGUI &pg = (PainterGUI&)p->device();
        pg.setColor(r,g,b,a);
        }

      i = pr;
      size_t pr2 = 0;
      if( (pr2 = prefix( txt, i, L"/>" )) != size_t(-1) )
        i = pr2;
      }
    else {
      Font::Leter l = font.leter(res, txt[i]);

      if( p ){
        p->setTexture( l.surf );
        p->drawRect( x+l.dpos.x+dx,
                     y+l.dpos.y+dy,
                     l.size.w, l.size.h,
                     0, 0, l.size.w, l.size.h );
        }

      w = std::max( w, x+l.advance.x );
      h = std::max( h, y+l.advance.y );

      x+= l.advance.x;
      y+= l.advance.y;
      ddy = std::max(ddy, l.dpos.y + l.size.h);
      }
    }

  rect = MyWidget::Size( w, h+ddy );
  }

void RichText::paintEvent( MyWidget::PaintEvent &e ) {
  MyWidget::Painter p(e);

  renderText( 0, 0, res, p, txt );
  }

size_t RichText::prefix( const std::wstring &txt,
                         size_t pos,
                         const wchar_t *pattern) {
  //size_t i = pos;
  if( pos<txt.size() && isSpace(txt[pos]) )
    return -1;

  for( size_t i=0; pattern[i] && pos<txt.size(); ++i ){
    while( pos<txt.size() && isSpace(txt[pos]) ){
      ++pos;
      }

    if( pos>=txt.size() )
      return -1;

    if( txt[pos]!=pattern[i] ){
      return -1;
      }

    ++pos;
    }

  return pos-1;
  }

bool RichText::isSpace(wchar_t c) {
  return c==' '  ||
         c=='\t' ||
         c=='\n' ||
         c=='\r';
  }

int RichText::cToInt(wchar_t a, wchar_t b) {
  int ia = 0,
      ib = 0;

  if( '0'<=a && a<='9' )
    ia = a-'0';
  if( 'A'<=a && a<='F' )
    ia = a-'A'+10;
  if( 'a'<=a && a<='f' )
    ia = a-'a'+10;

  if( '0'<=b && b<='9' )
    ib = b-'0';
  if( 'A'<=b && b<='F' )
    ib = b-'A'+10;
  if( 'a'<=b && b<='f' )
    ib = b-'a'+10;

  return ia*16+ib;
  }
