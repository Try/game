#ifndef HINTSYS_H
#define HINTSYS_H

#include <string>
#include <MyWidget/Utility>

class HintSys {
  public:
    static void setHint( const std::wstring& current,
                         const MyWidget::Rect & r );

    static const std::wstring& hint();
    static const MyWidget::Point pos();

    static int time;
    static const MyWidget::Rect& vrect();
  private:
    static std::wstring    current;
    static MyWidget::Rect     rect;
  };

#endif // HINTSYS_H
