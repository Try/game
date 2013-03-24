#ifndef PIXMAPSPOOL_H
#define PIXMAPSPOOL_H

#include <Tempest/TextureHolder>
#include <Tempest/Pixmap>
#include <Tempest/Texture2d>
#include <Tempest/Utility>
#include <vector>

class PixmapsPool {
    struct Page;
  public:
    PixmapsPool( Tempest::TextureHolder & h );

    struct TexturePtr{
      TexturePtr():tex(0), id(0), nonPool(0){}
      Tempest::Rect rect;
      std::vector<Page> * tex;
      size_t id;

      Tempest::Texture2d * nonPool;
      const Tempest::Texture2d& pageRawData() const;
      };
    TexturePtr add( const Tempest::Pixmap & p );

    void flush();
  private:
    Tempest::TextureHolder & holder;
    bool needToflush;

    struct Page{
      std::vector< Tempest::Rect > rects;
      Tempest::Texture2d t;
      Tempest::Pixmap    p;
      };

    std::vector<Page> page;

    void addPage();
    TexturePtr add( const Tempest::Pixmap & p, Page &page );

    static const int pageSize;
  };

#endif // PIXMAPSPOOL_H
