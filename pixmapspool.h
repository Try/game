#ifndef PIXMAPSPOOL_H
#define PIXMAPSPOOL_H

#include <MyGL/TextureHolder>
#include <MyGL/Pixmap>
#include <MyGL/Texture2d>
#include <MyWidget/Utility>
#include <vector>

class PixmapsPool {
    struct Page;
  public:
    PixmapsPool( MyGL::TextureHolder & h );

    struct TexturePtr{
      TexturePtr():tex(0), id(0), nonPool(0){}
      MyWidget::Rect rect;
      std::vector<Page> * tex;
      size_t id;

      MyGL::Texture2d * nonPool;
      const MyGL::Texture2d& pageRawData() const;
      };
    TexturePtr add( const MyGL::Pixmap & p );

    void flush();
  private:
    MyGL::TextureHolder & holder;
    bool needToflush;

    struct Page{
      std::vector< MyWidget::Rect > rects;
      MyGL::Texture2d t;
      MyGL::Pixmap    p;
      };

    std::vector<Page> page;

    void addPage();
    TexturePtr add( const MyGL::Pixmap & p, Page &page );
  };

#endif // PIXMAPSPOOL_H
