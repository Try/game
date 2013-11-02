#ifndef BUTTON_H
#define BUTTON_H

#include <Tempest/Widget>
#include <Tempest/Shortcut>
#include <Tempest/Sprite>
#include <Tempest/Font>

#include <ctime>

class Resource;

class Button : public Tempest::Widget {
  public:
    Button( Resource & res );

    Tempest::signal<> clicked;

    Tempest::Sprite frame, back[2], icon;

    void setBackTexture( const Tempest::Sprite & t );
    void setShortcut( const Tempest::Shortcut & sc );

    const std::wstring text() const;
    void setText( const std::wstring& t );
    void setText( const std::string& t );

    void setHint( const std::wstring & str );
    const std::wstring &hint() const;

    void setFont( const Tempest::Font& f );
    const Tempest::Font& font() const;

  protected:
    virtual void drawFrame(Tempest::Painter &p, const Tempest::Rect& r );
    virtual void drawFrame(Tempest::Painter &p);

    virtual void drawBack(Tempest::Painter &p, const Tempest::Rect& r );
    virtual void drawBack(Tempest::Painter &p);

    virtual Tempest::Rect viewRect() const;

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);
    void paintEvent( Tempest::PaintEvent &p);

    void gestureEvent(Tempest::AbstractGestureEvent &e);

    void keyPressEvent(Tempest::KeyEvent &e);

    void focusChange(bool);

    void onShortcut();
    virtual void emitClick();

    Resource & res;
    bool isPressed() const;
    void finishPaint();

  private:
    clock_t timePressed;
    bool pressed, presAnim;
    std::wstring txt, hnt;

    Tempest::Shortcut hotKey;

    Tempest::Font fnt;
  };

#endif // BUTTON_H
