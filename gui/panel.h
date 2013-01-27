#ifndef PANEL_H
#define PANEL_H

#include <MyWidget/Widget>
#include "graphics/paintergui.h"

class Resource;

class Panel : public MyWidget::Widget {
  public:
    typedef MyWidget::Bind::UserTexture Texture;
    Panel(Resource &res );

    Texture frame, back;

    void setDragable( bool d );
    bool isDragable();

  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseDragEvent(MyWidget::MouseEvent &e);
    void mouseMoveEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);

    void mouseWheelEvent(MyWidget::MouseEvent &e);

    void paintEvent( MyWidget::PaintEvent &p);

  private:
    bool mouseTracking, dragable;
    MyWidget::Point mpos, oldPos;
  };

#endif // PANEL_H
