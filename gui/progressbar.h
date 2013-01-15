#ifndef PROGRASSBAR_H
#define PROGRESSBAR_H

#include <MyWidget/Widget>
#include <MyWidget/Shortcut>
#include "graphics/paintergui.h"

#include <ctime>

class Resource;

class ProgressBar : public MyWidget::Widget {
  public:
    typedef MyWidget::Bind::UserTexture Texture;
    ProgressBar( Resource & res );

    MyWidget::signal<> clicked;

    Texture frame, back[2], icon;

    void setBackTexture( const Texture & t );
    void setShortcut( const MyWidget::Shortcut & sc );

    const std::wstring text() const;
    void setText( const std::wstring& t );
    void setText( const std::string& t );

    void setValue( int v );
    int  value() const;
    MyWidget::signal<int> onValueChanged;

    void setRange(int min, int max);
    int  min() const;
    int  max() const;
    MyWidget::signal<int, int> onRangeChanged;
  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseMoveEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);
    void paintEvent( MyWidget::PaintEvent &p);

    void keyPressEvent(MyWidget::KeyEvent &e);

    void focusChange(bool);
    std::wstring txt;

    int val, minV, maxV;
    MyWidget::Shortcut hotKey;
    Resource & res;
  };

#endif // PROGRASSBAR_H
