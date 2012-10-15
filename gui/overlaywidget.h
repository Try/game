#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <MyWidget/Widget>
#include <MyWidget/Layout>

class Resource;
class MainGui;

class OverlayWidget : public MyWidget::Widget {
  public:
    OverlayWidget( MainGui& mgui, Resource &r );

    void setupSignals();

    struct ContainerLayout : public MyWidget::Layout {
      void applyLayout();
      };
  protected:
    void focusEvent( bool f );
    MainGui & maingui;
  };

#endif // OVERLAYWIDGET_H
