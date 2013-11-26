#ifndef TEXTUREVIEW_H
#define TEXTUREVIEW_H

#include <Tempest/Texture2d>
#include <Tempest/Widget>

#include "panel.h"

class Resource;

class TextureView : public Tempest::Widget {
  public:
    TextureView( Resource & res );
    ~TextureView();

    void setTexture( const Tempest::Texture2d & t );

  protected:
    void paintEvent(Tempest::PaintEvent &e);

    bool alpha;
    Tempest::Texture2d texture;
  };

#endif // TEXTUREVIEW_H
