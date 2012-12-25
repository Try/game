#include "textureview.h"

#include <MyWidget/Painter>
#include "graphics/paintergui.h"

#include <iostream>

TextureView::TextureView(Resource &) {
  }

TextureView::~TextureView() {
  }

void TextureView::setTexture(const MyGL::Texture2d &t) {
  texture = t;
  }

void TextureView::paintEvent(MyWidget::PaintEvent &e) {
  MyWidget::Painter p(e);
  MyWidget::Bind::UserTexture u;

  u.data.nonPool = &texture;

  p.setTexture( u );
  p.drawRect( 0, 0, w(), h(),
              0, 0, texture.width(), texture.height() );
  }
