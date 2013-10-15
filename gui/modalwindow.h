#ifndef MODALWINDOW_H
#define MODALWINDOW_H

#include <Tempest/Widget>
#include <Tempest/Sprite>

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
    void closeEvent(Tempest::CloseEvent &e);
  private:
    Tempest::Sprite frame;
  };

#endif // MODALWINDOW_H
