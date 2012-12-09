#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <MyWidget/Widget>
#include <string>

#include "font.h"

class Resource;

class LineEdit : public MyWidget::Widget {
  public:
    LineEdit( Resource & res );

    void setText( const std::wstring& t );
    const std::wstring& text() const;

    MyWidget::signal<const std::wstring&> onTextChanged;
    MyWidget::signal<const std::wstring&> onTextEdited;
    MyWidget::signal<const std::wstring&> onEditingFinished;

    size_t selectionBegin();
    size_t selectionEnd();
    void setSelectionBounds( size_t begin, size_t end );

    void setEditable( bool e );
    bool isEditable() const;
  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);
    void mouseDragEvent(MyWidget::MouseEvent &e);

    void paintEvent(MyWidget::PaintEvent &p);

    void keyDownEvent(MyWidget::KeyEvent &e);

  private:
    std::wstring txt;
    Resource & res;

    bool editable;

    size_t sedit, eedit;
    MyWidget::Point sp, ep;
    int scrool;

    Font font;
    MyWidget::Bind::UserTexture frame;

    bool isEdited;
    void updateSel();

    void storeText(bool);
  };

#endif // LINEEDIT_H
