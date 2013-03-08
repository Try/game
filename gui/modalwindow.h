#ifndef MODALWINDOW_H
#define MODALWINDOW_H

#include <Tempest/Widget>
#include "graphics/paintergui.h"

class Resource;

class ModalWindow : public Tempest::Widget {
  public:
    ModalWindow(Resource &res, Tempest::Widget* owner );
    ~ModalWindow();

    Tempest::signal<> onClosed;
  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseWheelEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void paintEvent(Tempest::PaintEvent &e);

    void shortcutEvent(Tempest::KeyEvent &e);
  private:
    Tempest::Bind::UserTexture frame;
  };

#endif // MODALWINDOW_H
