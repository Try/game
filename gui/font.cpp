#include "font.h"

#include <Tempest/Pixmap>
#include "resource.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>

struct Tempest::Bind::UserFont::FreeTypeLib{
  FreeTypeLib(){
    FT_Init_FreeType( &library );
    }

  ~FreeTypeLib(){
    FT_Done_FreeType( library );

    std::map<Tempest::Bind::UserFont::Key, Tempest::Bind::UserFont::Leters*>::iterator
        b, e;
    b = Tempest::Bind::UserFont::letterBox.begin();
    e = Tempest::Bind::UserFont::letterBox.end();

    for( ; b!=e; ++b )
      delete b->second;
    }

  FT_Library    library;
  };

std::map<Tempest::Bind::UserFont::Key, Tempest::Bind::UserFont::Leters*>
  Tempest::Bind::UserFont::letterBox;

Tempest::Bind::UserFont::FreeTypeLib& Tempest::Bind::UserFont::ft(){
  static FreeTypeLib lib;
  return lib;
  }

Tempest::Bind::UserFont::UserFont( const std::string &name, int sz)  {
  key.name     = name+".ttf";
  key.baseName = name;

  key.size = sz;

  key.bold   = false;
  key.italic = false;

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }

Tempest::Bind::UserFont::UserFont(int sz) {
#ifdef __ANDROID__
  key.name = "/system/fonts/DroidSans.ttf";
  key.baseName = "/system/fonts/DroidSans";
#else
  key.name = "./data/arial.ttf";
  key.baseName = "./data/arial";
#endif

  key.size = sz;
  key.bold   = false;
  key.italic = false;

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }

Tempest::Bind::UserFont::UserFont() {
#ifdef __ANDROID__
  key.name = "/system/fonts/DroidSans.ttf";
  key.baseName = "/system/fonts/DroidSans";
#else
  key.name = "./data/arial.ttf";
  key.baseName = "./data/arial";
#endif

  key.size = 16;
  key.bold   = false;
  key.italic = false;

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }

const Tempest::Bind::UserFont::Leter&
    Tempest::Bind::UserFont::fetchLeter( Resource &res, wchar_t ch ) const {
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

  letter.dpos = Tempest::Point( slot->bitmap_left,
                                 key.size - slot->bitmap_top );

  Tempest::Pixmap pixmap( bmap.width, bmap.rows, true );

  for( int i=0; i<pixmap.width(); ++i )
    for( int r=0; r<pixmap.height(); ++r ){
      uint8_t lum = bmap.buffer[r * bmap.width + i];
      Tempest::Pixmap::Pixel p = {255, 255, 255, lum};
      pixmap.set( i,r, p );
      }

  //pixmap.save("./l.png");
  letter.surf.data = res.pixmap( pixmap, false );
  letter.size      = Tempest::Size( pixmap.width(), pixmap.height() );
  letter.advance   = Tempest::Point( slot->advance.x >> 6,
                                      slot->advance.y >> 6 );

  FT_Done_Face( face );

  Leter &ref = leters[ch];
  ref = letter;
  return ref;
  }

void Tempest::Bind::UserFont::fetch( Resource &res,
                                      const std::wstring &str ) const {
  for( size_t i=0; i<str.size(); ++i )
    fetchLeter( res, str[i] );
  }

Tempest::Size Tempest::Bind::UserFont::textSize( Resource &res,
                                                   const std::wstring & str ) {
  int tx = 0, ty = 0, tw = 0, th = 0;
  for( size_t i=0; i<str.size(); ++i ){
    const Font::Leter& l = leter( res, str[i] );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }

  return Tempest::Size(tw,th);
  }

int Tempest::Bind::UserFont::size() const {
  return key.size;
  }

void Tempest::Bind::UserFont::setBold(bool b) {
  key.bold = b;
  update();
  }

bool Tempest::Bind::UserFont::isBold() const {
  return key.bold;
  }

void Tempest::Bind::UserFont::setItalic(bool b) {
  key.italic = b;
  update();
  }

bool Tempest::Bind::UserFont::isItalic() const {
  return key.italic;
  }

void Tempest::Bind::UserFont::setSize(int s) {
  key.size = s;
  update();
  }

const Tempest::Bind::UserFont::Leter&
  Tempest::Bind::UserFont::leter(Resource &res, wchar_t ch) const {
  const Tempest::Bind::UserFont::Leter& tmp = fetchLeter(res, ch);
  res.flushPixmaps();
  return tmp;
  }

void Tempest::Bind::UserFont::update() {
  key.name = key.baseName;

  if( key.bold )
    key.name +=  "b";

  if( key.italic )
    key.name +=  "i";

  key.name +=  ".ttf";

  lt = letterBox[key];
  if( !lt ){
    lt = new Leters();
    letterBox[key] = lt;
    }
  }
