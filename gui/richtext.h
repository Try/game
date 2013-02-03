#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <MyWidget/Widget>
#include <string>

class Resource;

namespace MyWidget{
  class Painter;
  }

class RichText : public MyWidget::Widget {
  public:
    RichText( Resource & res );

    static void renderText(int x, int y, Resource &res, MyWidget::Painter & p,
                            const std::wstring& txt );
    static MyWidget::Size bounds( Resource & res,
                                  const std::wstring& txt );
  protected:
    void paintEvent(MyWidget::PaintEvent &e);

  private:
    static void rText( int dx,
                       int dy,
                       Resource & res,
                       MyWidget::Painter *p,
                       const std::wstring& txt,
                       MyWidget::Size &rect );

    std::wstring txt;
    Resource & res;

    static size_t prefix( const std::wstring& txt,
                          size_t pos,
                          const wchar_t * pattern );
    static bool isSpace( wchar_t c );
    static int  cToInt ( wchar_t a, wchar_t b );
  };

#endif // RICHTEXT_H
