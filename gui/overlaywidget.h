#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <Tempest/Widget>
#include <Tempest/Layout>

class Resource;
class MainGui;

class OverlayWidget : public Tempest::Widget {
  public:
    OverlayWidget();

    void setupSignals();

    struct ContainerLayout : public Tempest::Layout {
      ContainerLayout();
      void applyLayout();
      bool used;
      };
  protected:
    void focusEvent( bool f );

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseWheelEvent(Tempest::MouseEvent &e);
  };

#endif // OVERLAYWIDGET_H
