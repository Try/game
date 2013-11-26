#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <Tempest/Widget>
#include <string>

class Resource;

namespace Tempest{
  class Painter;
  }

class RichText : public Tempest::Widget {
  public:
    RichText( Resource & res );

    static void renderText( int x, int y,
                            Tempest::Painter & p,
                            const std::wstring& txt );
    static void renderText( int x, int y,
                            int w, int h,
                            Tempest::Painter & p,
                            const std::wstring& txt );
    static Tempest::Size bounds(const std::wstring& txt );
    static Tempest::Size bounds( Tempest::Rect& destRect, const std::wstring& txt );

    void setText( const std::wstring & text );
  protected:
    void paintEvent(Tempest::PaintEvent &e);

  private:    
    struct Char2Draw {
      Tempest::Color color;
      Tempest::Rect  rect;
      Tempest::Font::Letter c;
      };
    static std::vector<Char2Draw> buffer;

    static void rText( int dx, int dy,
                       int  w, int  h,
                       std::vector<Char2Draw> &buffer,
                       Tempest::Painter *p,
                       const std::wstring& txt,
                       Tempest::Size &rect );

    static void reposNextLine(int &x, int &y, int dx, int dw, int ddy);

    std::wstring txt;
    Resource & res;

    static size_t prefix( const std::wstring& txt,
                          size_t pos,
                          const wchar_t * pattern );
    static bool isSpace( wchar_t c );
    static int  cToInt ( wchar_t a, wchar_t b );
  };

#endif // RICHTEXT_H
