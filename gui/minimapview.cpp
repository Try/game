#include "minimapview.h"

#include "resource.h"

#include "game/world.h"

MiniMapView::MiniMapView( Resource &res ):TextureView(res), res(res) {
  rtime = clock();
  }

void MiniMapView::render(World &wx) {
  if( renderTo.width() != w() ||
      renderTo.height()!= h() ){
    renderTo = MyGL::Pixmap(w(), h(), false);
    rtime = clock() - CLOCKS_PER_SEC/2;
    }

  if( clock() >= rtime+CLOCKS_PER_SEC/2 ){
    rtime = clock();

    MyGL::Pixmap::Pixel pix;
    pix.r = 0;
    pix.g = 0;
    pix.b = 0;
    pix.a = 255;

    for( int i=0; i<renderTo.width(); ++i )
      for( int r=0; r<renderTo.height(); ++r ){
        int terrX = (i*wx.terrain().width()) /renderTo.width();
        int terrY = (r*wx.terrain().height())/renderTo.height();

        pix.r = 0;
        pix.g = 0;
        pix.b = 0;

        if( wx.terrain().isEnable(terrX,terrY) ){
          int h = wx.terrain().heightAt( terrX, terrY );
          h = std::max(0, std::min(h, 64) );
          pix.g = 90+h;
          } else {
          if( wx.terrain().at(terrX, terrY) -
              wx.terrain().heightAt( terrX, terrY ) > 0 ){
            pix.b = 255;
            pix.g = 128;
            }
          }

        renderTo.set(i,r, pix);
        }

    for( size_t i=0; i<wx.objectsCount(); ++i ){
      GameObject &obj = wx.object(i);
      int terrX = (obj.x()+Terrain::quadSize)/Terrain::quadSize;
      int terrY = (obj.y()+Terrain::quadSize)/Terrain::quadSize;

      terrX = (terrX*renderTo.width()) /wx.terrain().width();
      terrY = (terrY*renderTo.height())/wx.terrain().height();

      pix.r = obj.teamColor().r()*255;
      pix.g = obj.teamColor().g()*255;
      pix.b = obj.teamColor().b()*255;

      if( terrX >=0 && terrX<renderTo.width() &&
          terrY >=0 && terrY<renderTo.height() )

        if( !obj.getClass().data.isBackground ){
          int sz = obj.getClass().data.size;

          int terrX0 = (obj.x())/Terrain::quadSize - sz/2;
          int terrY0 = (obj.y())/Terrain::quadSize - sz/2;
          terrX0 = (terrX0*renderTo.width()) /wx.terrain().width();
          terrY0 = (terrY0*renderTo.height())/wx.terrain().height();

          int terrX1 = (obj.x())/Terrain::quadSize + sz-sz/2;
          int terrY1 = (obj.y())/Terrain::quadSize + sz-sz/2;
          terrX1 = (terrX1*renderTo.width()) /wx.terrain().width();
          terrY1 = (terrY1*renderTo.height())/wx.terrain().height();

          terrX0 = std::max(terrX0,0);
          terrY0 = std::max(terrY0,0);
          terrX1 = std::min(terrX1,renderTo.width()-1);
          terrY1 = std::min(terrY1,renderTo.height()-1);

          for( int x=terrX0; x<terrX1; ++x )
            for( int y=terrY0; y<terrY1; ++y ){
              renderTo.set(x, y, pix);
              }
          }
      }

    setTexture( res.ltexHolder.create( renderTo, 0) );
    }
  }
