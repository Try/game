#ifndef INGAMEMENU_H
#define INGAMEMENU_H

#include "modalwindow.h"
#include "graphics/paintergui.h"

class InGameMenu : public ModalWindow {
  public:
    InGameMenu( Resource & res, MyWidget::Widget* );

  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void paintEvent(MyWidget::PaintEvent &p);
  private:
    MyWidget::Bind::UserTexture frame;
  };

#endif // INGAMEMENU_H
