#ifndef MODALWINDOW_H
#define MODALWINDOW_H

#include <MyWidget/Widget>

class Resource;

class ModalWindow : public MyWidget::Widget {
  public:
    ModalWindow( Resource & res, MyWidget::Widget* owner );
    ~ModalWindow();

    MyWidget::signal<> onClosed;
  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseWheelEvent(MyWidget::MouseEvent &e);
  };

#endif // MODALWINDOW_H
