#include "font.h"

#include <MyGL/Pixmap>
#include "resource.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>

struct MyWidget::Bind::UserFont::FreeTypeLib{
  FreeTypeLib(){
    FT_Init_FreeType( &library );
    }

  ~FreeTypeLib(){
    FT_Done_FreeType( library );

    std::map<MyWidget::Bind::UserFont::Key, MyWidget::Bind::UserFont::Leters*>::iterator
        b, e;
    b = MyWidget::Bind::UserFont::letterBox.begin();
    e = MyWidget::Bind::UserFont::letterBox.end();

    for( ; b!=e; ++b )
      delete b->second;
    }

  FT_Library    library;
  };

std::map<MyWidget::Bind::UserFont::Key, MyWidget::Bind::UserFont::Leters*>
  MyWidget::Bind::UserFont::letterBox;

MyWidget::Bind::UserFont::FreeTypeLib& MyWidget::Bind::UserFont::ft(){
  static FreeTypeLib lib;
  return lib;
  }

MyWidget::Bind::UserFont::UserFont( const std::string &name, int sz)  {
  key.name = name;
  key.size = sz;

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }

MyWidget::Bind::UserFont::UserFont(int sz) {
  key.name = "./data/arial.ttf";
  key.size = sz;

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }

MyWidget::Bind::UserFont::UserFont() {
  key.name = "./data/arial.ttf";
  key.size = 16;

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }

const MyWidget::Bind::UserFont::Leter&
    MyWidget::Bind::UserFont::fetchLeter( Resource &res, wchar_t ch ) const {
  Leters & leters = *lt;

  std::unordered_map< wchar_t, Leter >::iterator it = leters.find(ch);
  if( it != leters.end() )
    return it->second;


  FT_Face       face;
  FT_Vector     pen = {};
  FT_Error err = 0;
  err = FT_New_Face( ft().library, key.name.c_str(), 0, &face );

  err = FT_Set_Pixel_Sizes( face, key.size, key.size );
  FT_Set_Transform( face, 0, &pen );

  Leter letter;
  if( FT_Load_Char( face, ch, FT_LOAD_RENDER ) ){
    Leter &ref = leters[ch];
    ref = letter;
    ref.surf.data.tex = 0;
    return ref;
    }

  FT_GlyphSlot slot = face->glyph;
  FT_Bitmap& bmap = slot->bitmap;

  letter.dpos = MyWidget::Point( slot->bitmap_left,
                                 key.size - slot->bitmap_top );

  MyGL::Pixmap pixmap( bmap.width, bmap.rows, true );

  for( int i=0; i<pixmap.width(); ++i )
    for( int r=0; r<pixmap.height(); ++r ){
      uint8_t lum = bmap.buffer[r * bmap.width + i];
      MyGL::Pixmap::Pixel p = {255, 255, 255, lum};
      pixmap.set( i,r, p );
      }

  //pixmap.save("./l.png");
  letter.surf.data = res.pixmap( pixmap, false );
  letter.size = MyWidget::Size( pixmap.width(), pixmap.height() );
  letter.advance = MyWidget::Point( slot->advance.x >> 6,
                                    slot->advance.y >> 6 );

  FT_Done_Face    ( face );

  Leter &ref = leters[ch];
  ref = letter;
  return ref;
  }

void MyWidget::Bind::UserFont::fetch( Resource &res,
                                      const std::wstring &str ) const {
  for( size_t i=0; i<str.size(); ++i )
    fetchLeter( res, str[i] );
  }

MyWidget::Size MyWidget::Bind::UserFont::textSize( Resource &res,
                                                   const std::wstring & str ) {
  int tx = 0, ty = 0, tw = 0, th = 0;
  for( size_t i=0; i<str.size(); ++i ){
    const Font::Leter& l = leter( res, str[i] );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }

  return MyWidget::Size(tw,th);
  }

int MyWidget::Bind::UserFont::size() const {
  return key.size;
  }

const MyWidget::Bind::UserFont::Leter&
  MyWidget::Bind::UserFont::leter(Resource &res, wchar_t ch) const {
  const MyWidget::Bind::UserFont::Leter& tmp = fetchLeter(res, ch);
  res.flushPixmaps();
  return tmp;
  }
