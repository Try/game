#ifndef BUTTON_H
#define BUTTON_H

#include <Tempest/Widget>
#include <Tempest/Shortcut>
#include "graphics/paintergui.h"
#include "font.h"

#include <ctime>

class Resource;

class Button : public Tempest::Widget {
  public:
    typedef Tempest::Bind::UserTexture Texture;
    Button( Resource & res );

    Tempest::signal<> clicked;

    Texture frame, back[2], icon;

    void setBackTexture( const Texture & t );
    void setShortcut( const Tempest::Shortcut & sc );

    const std::wstring text() const;
    void setText( const std::wstring& t );
    void setText( const std::string& t );

    void setHint( const std::wstring & str );
    const std::wstring &hint() const;
  protected:
    virtual void drawFrame(Tempest::Painter &p, const Tempest::Rect& r );
    virtual void drawFrame(Tempest::Painter &p);

    virtual void drawBack(Tempest::Painter &p, const Tempest::Rect& r );
    virtual void drawBack(Tempest::Painter &p);

    virtual Tempest::Rect viewRect() const;

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void paintEvent( Tempest::PaintEvent &p);

    void keyPressEvent(Tempest::KeyEvent &e);

    void focusChange(bool);
    bool pressed, presAnim;
    std::wstring txt, hnt;

    Tempest::Shortcut hotKey;
    Resource & res;

    Font font;

    void onShortcut();
    virtual void emitClick();

    clock_t timePressed;
  };

#endif // BUTTON_H
