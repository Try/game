#ifndef MODALWINDOW_H
#define MODALWINDOW_H

#include <MyWidget/Widget>
#include "graphics/paintergui.h"

class Resource;

class ModalWindow : public MyWidget::Widget {
  public:
    ModalWindow(Resource &res, MyWidget::Widget* owner );
    ~ModalWindow();

    MyWidget::signal<> onClosed;
  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseWheelEvent(MyWidget::MouseEvent &e);
    void mouseMoveEvent(MyWidget::MouseEvent &e);

    void paintEvent(MyWidget::PaintEvent &e);

    void shortcutEvent(MyWidget::KeyEvent &e);
  private:
    MyWidget::Bind::UserTexture frame;
  };

#endif // MODALWINDOW_H
