#ifndef PROGRASSBAR_H
#define PROGRESSBAR_H

#include <Tempest/Widget>
#include <Tempest/Shortcut>
#include <Tempest/Sprite>
#include "graphics/paintergui.h"

#include <ctime>

class Resource;

class ProgressBar : public Tempest::Widget {
  public:
    ProgressBar( Resource & res );

    Tempest::signal<> clicked;

    Tempest::Sprite frame, back[2], icon;

    void setBackTexture( const Tempest::Sprite & t );
    void setShortcut( const Tempest::Shortcut & sc );

    const std::wstring text() const;
    void setText( const std::wstring& t );
    void setText( const std::string& t );

    void setValue( int v );
    int  value() const;
    Tempest::signal<int> onValueChanged;

    void setRange(int min, int max);
    int  min() const;
    int  max() const;
    Tempest::signal<int, int> onRangeChanged;
  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void paintEvent( Tempest::PaintEvent &p);

    void keyPressEvent(Tempest::KeyEvent &e);

    void focusChange(bool);
    std::wstring txt;

    int val, minV, maxV;
    Tempest::Shortcut hotKey;
    Resource & res;
  };

#endif // PROGRASSBAR_H
