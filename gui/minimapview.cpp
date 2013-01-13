#include "minimapview.h"

#include "resource.h"

#include "game/world.h"

MiniMapView::MiniMapView( Resource &res ):TextureView(res), res(res) {
  rtime   = clock();
  pressed = false;
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
    cashed = renderTo;
    }


  renderTo = cashed;
  MyGL::Pixmap::Pixel pix;
  pix.r = 255;
  pix.g = 255;
  pix.b = 255;
  pix.a = 255;

  World::CameraViewBounds b = wx.cameraBounds();
  int sx = wx.terrain().width()*Terrain::quadSize,
      sy = wx.terrain().width()*Terrain::quadSize;

  for( int i=0; i<4; ++i ){
    b.x[i] = (b.x[i]*renderTo.width()) /sx;
    b.y[i] = (b.y[i]*renderTo.height())/sy;
    }

  lineTo( renderTo, b.x[0], b.y[0], b.x[1], b.y[1] );
  lineTo( renderTo, b.x[0], b.y[0], b.x[2], b.y[2] );
  lineTo( renderTo, b.x[1], b.y[1], b.x[3], b.y[3] );
  lineTo( renderTo, b.x[3], b.y[3], b.x[2], b.y[2] );

  setTexture( MyGL::Texture2d() );
  setTexture( res.ltexHolder.create( renderTo, 0) );
  }

void MiniMapView::lineTo( MyGL::Pixmap &renderTo,
                          int x0, int y0,
                          int x1, int y1) {
  MyGL::Pixmap::Pixel pix;
  pix.r = 255;
  pix.g = 255;
  pix.b = 255;
  pix.a = 255;

  if( x0==x1 && 0<=x1 && x1<renderTo.width() ){
    if( y0>y1 )
      std::swap(y0,y1);

    y0 = std::max(0,y0);
    y1 = std::min(renderTo.height()-1,y1+1);
    for( int i=y0; i<y1; ++i )
      renderTo.set(x0, i, pix);

    return;
    }

  if( y0==y1 && 0<=y1 && y1<renderTo.height() ){
    if( x0>x1 )
      std::swap(x0,x1);

    x0 = std::max(0,x0);
    x1 = std::min(renderTo.width()-1,x1+1);
    for( int i=x0; i<x1; ++i )
      renderTo.set(i, y0, pix);

    return;
    }

  const int deltaX = abs(x1 - x0);
  const int deltaY = abs(y1 - y0);
  const int signX = x0 < x1 ? 1 : -1;
  const int signY = y0 < y1 ? 1 : -1;

  int error = deltaX - deltaY;

  if( 0<=x1 && x1<renderTo.width() &&
      0<=y1 && y1<renderTo.height() )
    renderTo.set(x1, y1, pix);

  while(x0 != x1 || y0 != y1) {
    if( 0<=x0 && x0<renderTo.width() &&
        0<=y0 && y0<renderTo.height() )
      renderTo.set(x0, y0, pix);

    const int error2 = error * 2;

    if(error2 > -deltaY) {
      error -= deltaY;
      x0 += signX;
      }
    if(error2 < deltaX) {
      error += deltaX;
      y0 += signY;
      }
    }
  }

void MiniMapView::mouseDownEvent(MyWidget::MouseEvent &e) {
  mouseEvent( e.x/float(w()), e.y/float(h()) );
  pressed = true;
  }

void MiniMapView::mouseDragEvent(MyWidget::MouseEvent &e) {
  mouseEvent( e.x/float(w()), e.y/float(h()) );
  }

void MiniMapView::mouseUpEvent(MyWidget::MouseEvent &e) {
  mouseEvent( e.x/float(w()), e.y/float(h()) );
  pressed = false;
  }
