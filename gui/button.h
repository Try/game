#ifndef BUTTON_H
#define BUTTON_H

#include <MyWidget/Widget>
#include <MyWidget/Shortcut>
#include "graphics/paintergui.h"

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
  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseMoveEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);
    void paintEvent( MyWidget::PaintEvent &p);

    void keyPressEvent(MyWidget::KeyEvent &e);

    void focusChange(bool);
    bool pressed, presAnim;
    std::wstring txt;

    MyWidget::Shortcut hotKey;
    Resource & res;
  };

#endif // BUTTON_H
