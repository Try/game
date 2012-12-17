#ifndef MODALWINDOW_H
#define MODALWINDOW_H

#include <MyWidget/Widget>

class Resource;

class ModalWindow : public MyWidget::Widget {
  public:
    ModalWindow( Resource & res, MyWidget::Widget* owner );

  protected:
    void mouseWheelEvent(MyWidget::MouseEvent &e);
  };

#endif // MODALWINDOW_H
