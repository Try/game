#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <MyWidget/Widget>

class Resource;
class MainGui;

class OverlayWidget : public MyWidget::Widget {
  public:
    OverlayWidget( MainGui& mgui, Resource &r );

  protected:
    void focusEvent( bool f );
    MainGui & maingui;
  };

#endif // OVERLAYWIDGET_H
