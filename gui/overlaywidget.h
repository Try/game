#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <MyWidget/Widget>
#include <MyWidget/Layout>

class Resource;
class MainGui;

class OverlayWidget : public MyWidget::Widget {
  public:
    OverlayWidget( Resource &r );

    void setupSignals();

    struct ContainerLayout : public MyWidget::Layout {
      ContainerLayout();
      void applyLayout();
      bool used;
      };
  protected:
    void focusEvent( bool f );

    void mouseDownEvent(MyWidget::MouseEvent &e);
  };

#endif // OVERLAYWIDGET_H
