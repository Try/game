#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <Tempest/Widget>
#include <Tempest/Font>
#include <Tempest/Timer>
#include <string>

class Resource;

class LineEdit : public Tempest::Widget {
  public:
    LineEdit( Resource & res );

    void setText( const std::wstring& t );
    const std::wstring& text() const;

    Tempest::signal<const std::wstring&> onTextChanged;
    Tempest::signal<const std::wstring&> onTextEdited;
    Tempest::signal<const std::wstring&> onEditingFinished;

    size_t selectionBegin();
    size_t selectionEnd();
    void setSelectionBounds( size_t begin, size_t end );

    void setEditable( bool e );
    bool isEditable() const;
  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);

    void paintEvent(Tempest::PaintEvent &p);

    void keyDownEvent(Tempest::KeyEvent &e);

  private:
    std::wstring txt;
    Resource & res;

    bool editable;
    bool anim;

    size_t sedit, eedit;
    Tempest::Point sp, ep;
    int scrool;

    Tempest::Font   font;
    Tempest::Sprite frame;
    Tempest::Timer  timer;

    bool isEdited;
    void updateSel();

    void storeText(bool);
    void setupTimer( bool );

    void animation();
  };

#endif // LINEEDIT_H
