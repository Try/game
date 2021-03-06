#include "pixmapspool.h"

#include <cassert>
#include <string>

#include <Tempest/Device>

/*
#ifdef __ANDROID__
const int PixmapsPool::pageSize = 2048;
#else
const int PixmapsPool::pageSize = 1024;
#endif
*/

PixmapsPool::PixmapsPool(Tempest::TextureHolder &h):holder(h) {
  needToflush = true;
  pageSize    = std::min( h.device().caps().maxTextureSize, 2048 );
  addPage();
  }

PixmapsPool::TexturePtr PixmapsPool::add(const Tempest::Pixmap &p) {
  if( p.width()==0 || p.height()==0 ){
    TexturePtr n;
    n.tex = 0;
    n.id  = 0;
    return n;
    }

  if( p.width()  > pageSize ||
      p.height() > pageSize ){
    Page pg;
    pg.p = p;

    page.push_back( pg );

    TexturePtr n;
    n.tex = &page;
    n.id  = page.size()-1;
    return n;
    }

  for( size_t i=0; i<page.size(); ++i ){
    PixmapsPool::TexturePtr r = add( p, page[i] );
    if( r.tex ){
      r.id  = i;
      r.tex = &page;

      return r;
      }

    if( i+1==page.size() )
      addPage();
    }

  return PixmapsPool::TexturePtr();
  }

void PixmapsPool::flush() {
  if( needToflush ){
    for( size_t i=0; i<page.size(); ++i ){
      page[i].t = holder.create( page[i].p, false, false );

      Tempest::Texture2d::Sampler s;
      s.mipFilter = Tempest::Texture2d::FilterType::Nearest;
      s.magFilter = s.mipFilter;
      s.minFilter = s.mipFilter;
      s.anisotropic = false;

      page[i].t.setSampler(s);
      //std::string str = "./debug*.png";
      //str[7] = i+'0';
      // page[i].p.save( str );
      }

    //page[0].p.save("./debug.png");
    }
  needToflush = false;
  }

void PixmapsPool::addPage() {
  Page p;
  p.p = Tempest::Pixmap(pageSize, pageSize, true);
  p.rects.push_back( Tempest::Rect(0,0, p.p.width(), p.p.height() ) );

  page.push_back( p );
  }

PixmapsPool::TexturePtr PixmapsPool::add(const Tempest::Pixmap &px, Page & page ) {
  needToflush = true;

  page.t = Tempest::Texture2d();

  Tempest::PixEditor p( page.p );

  size_t id = 0, sq = 0;
  for( size_t i=0; i<page.rects.size(); ++i ){
    const Tempest::Rect r = page.rects[i];

    if( r.w>=px.width() && r.h>=px.height() ){
      size_t sq2 = r.w*r.h;
      if( sq==0 || sq>sq2 ){
        id = i;
        sq = sq2;
        }
      }
    }

  //assert( sq!=0 );
  if( sq==0 ){
    TexturePtr n;
    n.tex = 0;
    return n;
    }

  Tempest::Rect r = page.rects[id];
  p.copy( r.x, r.y, px );

  if( r.w!=px.width() && r.h!=px.height() ){
    page.rects[id] = Tempest::Rect( r.x+px.width(), r.y+px.height(),
                                     r.w-px.width(), r.h-px.height() );
    } else {
    page.rects[id] = page.rects.back();
    page.rects.pop_back();
    }


  if( r.w!=px.width() )
    page.rects.push_back( Tempest::Rect( r.x+px.width(), r.y,
                                          r.w-px.width(), px.height() ) );

  if( r.h!=px.height() )
    page.rects.push_back( Tempest::Rect( r.x, r.y+px.height(),
                                          px.width(), r.h-px.height() ) );


  TexturePtr n;
  n.rect = Tempest::Rect( r.x, r.y, px.width(), px.height() );
  n.tex = &this->page;

  return n;
  }

const Tempest::Texture2d &PixmapsPool::TexturePtr::pageRawData() const {
  return (*tex)[id].t;
  }
