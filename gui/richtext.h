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

    static void renderText( int x, int y, Resource &res,
                            Tempest::Painter & p,
                            const std::wstring& txt );
    static Tempest::Size bounds( Resource & res,
                                  const std::wstring& txt );

    void setText( const std::wstring & text );
  protected:
    void paintEvent(Tempest::PaintEvent &e);

  private:
    static void rText( int dx,
                       int dy,
                       Resource & res,
                       Tempest::Painter *p,
                       const std::wstring& txt,
                       Tempest::Size &rect );

    std::wstring txt;
    Resource & res;

    static size_t prefix( const std::wstring& txt,
                          size_t pos,
                          const wchar_t * pattern );
    static bool isSpace( wchar_t c );
    static int  cToInt ( wchar_t a, wchar_t b );
  };

#endif // RICHTEXT_H
