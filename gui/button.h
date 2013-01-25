#ifndef BUTTON_H
#define BUTTON_H

#include <MyWidget/Widget>
#include <MyWidget/Shortcut>
#include "graphics/paintergui.h"

#include <ctime>

class Resource;

class Button : public MyWidget::Widget {
  public:
    typedef MyWidget::Bind::UserTexture Texture;
    Button( Resource & res );

    MyWidget::signal<> clicked;

    Texture frame, back[2], icon;

    void setBackTexture( const Texture & t );
    void setShortcut( const MyWidget::Shortcut & sc );

    const std::wstring text() const;
    void setText( const std::wstring& t );
    void setText( const std::string& t );

    void setHint( const std::wstring & str );
    const std::wstring &hint() const;
  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseMoveEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);
    void paintEvent( MyWidget::PaintEvent &p);

    void keyPressEvent(MyWidget::KeyEvent &e);

    void focusChange(bool);
    bool pressed, presAnim;
    std::wstring txt, hnt;

    MyWidget::Shortcut hotKey;
    Resource & res;

    void onShortcut();

    clock_t timePressed;
  };

#endif // BUTTON_H
