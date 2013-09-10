#include "textureview.h"

#include <Tempest/Painter>

#include <iostream>

TextureView::TextureView(Resource &) {
  }

TextureView::~TextureView() {
  }

void TextureView::setTexture(const Tempest::Texture2d &t) {
  texture = t;
  }

void TextureView::paintEvent(Tempest::PaintEvent &e) {
  Tempest::Painter p(e);

  p.setFlip(false, true);
  p.setTexture( texture );
  p.drawRect( 0, 0, w(), h(),
              0, 0, texture.width(), texture.height() );

  Widget::paintEvent(e);
  }
