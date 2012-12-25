#ifndef TEXTUREVIEW_H
#define TEXTUREVIEW_H

#include <MyGL/Texture2d>
#include <MyWidget/Widget>

#include "panel.h"

class Resource;

class TextureView : public MyWidget::Widget {
  public:
    TextureView( Resource & res );
    ~TextureView();

    void setTexture( const MyGL::Texture2d & t );

  protected:
    void paintEvent(MyWidget::PaintEvent &e);

    MyGL::Texture2d texture;
  };

#endif // TEXTUREVIEW_H
