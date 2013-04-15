#include "minimapview.h"

#include "resource.h"

#include "game/world.h"
#include "game.h"

MiniMapView::MiniMapView( Resource &res ):TextureView(res), res(res) {
  rtime   = clock();
  rtime2  = clock();
  pressed = false;
  }

void MiniMapView::render( World &wx ) {
  int mk = wx.terrain().width()*wx.terrain().height()/(128*128);

  if( terr.width() != w() ||
      terr.height()!= h() ){
    rtime = clock() - mk*CLOCKS_PER_SEC*2;
    update();
    }

  if( clock() >= rtime2+mk*CLOCKS_PER_SEC*2 ){
    rtime2 = clock();

    Tempest::Pixmap terr = Tempest::Pixmap(w(), h(), true);
    Tempest::Pixmap::Pixel pix;
    pix.r = 0;
    pix.g = 0;
    pix.b = 0;
    pix.a = 255;

    int tw = wx.terrain().width(),
        th = wx.terrain().height();

    for( int i=0; i<terr.width(); ++i )
      for( int r=0; r<terr.height(); ++r ){
        int terrX = (i*tw) /terr.width();
        int terrY = (r*th)/terr.height();

        pix.r = 0;
        pix.g = 0;
        pix.b = 0;

        if( wx.terrain().depthAt(terrX, terrY) > World::coordCastD(0.2) ){
          pix.b = 255;
          pix.g = 128;
          } else {
          float n[3] = {};
          wx.terrain().normalAt(terrX,terrY,n);
          float l = n[2];
          int h = 165;

          if( wx.terrain().isEnable(terrX,terrY) ){
            h += std::max(0, std::min(int(l*90), 90) );
            } else {
            h += std::max(0, std::min(int(l*20), 20) );
            }

          Tempest::Color cl = wx.terrain().colorAt(terrX, terrY);
          pix.r = cl.r()*h;
          pix.g = cl.g()*h;
          pix.b = cl.b()*h;
          //pix.g = 50+h;
          }

        terr.set(i,r, pix);
        }

    this->terr = res.ltexHolder.create(terr, false, false);
    }


  if( clock() >= rtime+mk*CLOCKS_PER_SEC/2 ){
    rtime = clock();
    Tempest::Pixmap::Pixel pix;
    pix.r = 0;
    pix.g = 0;
    pix.b = 0;
    pix.a = 0;

    Tempest::Pixmap renderTo = Tempest::Pixmap(w(), h(), true);
    for( int i=0; i<terr.width(); ++i )
      for( int r=0; r<terr.height(); ++r )
        renderTo.set(i,r, pix);

    drawUnits(renderTo, wx);
    //aceptFog(renderTo, wx.game.player().fog() );
    units = res.ltexHolder.create(renderTo, false, false);

    Tempest::Pixmap fogTex = Tempest::Pixmap(w(), h(), true);
    aceptFog(fogTex, wx.game.player().fog() );

    fog   = res.ltexHolder.create(fogTex, false, false);
    }

  static Tempest::Pixmap hudPx = Tempest::Pixmap(w(), h(), true);
  Tempest::Pixmap::Pixel px = {};

  for( int i=0; i<hudPx.width(); ++i )
    for( int r=0; r<hudPx.height(); ++r ){
      hudPx.set(i,r, px );
      }

  World::CameraViewBounds b = wx.cameraBounds();
  int sx = wx.terrain().width()*Terrain::quadSize,
      sy = wx.terrain().width()*Terrain::quadSize;

  for( int i=0; i<4; ++i ){
    b.x[i] = (b.x[i]*hudPx.width()) /sx;
    b.y[i] = (b.y[i]*hudPx.height())/sy;
    }

  lineTo( hudPx, b.x[0], b.y[0], b.x[1], b.y[1] );
  lineTo( hudPx, b.x[0], b.y[0], b.x[2], b.y[2] );
  lineTo( hudPx, b.x[1], b.y[1], b.x[3], b.y[3] );
  lineTo( hudPx, b.x[3], b.y[3], b.x[2], b.y[2] );
  hud   = res.ltexHolder.create(hudPx, false, false);
  }

void MiniMapView::lineTo( Tempest::Pixmap &renderTo,
                          int x0, int y0,
                          int x1, int y1) {
  Tempest::Pixmap::Pixel pix;
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

void MiniMapView::mouseDownEvent(Tempest::MouseEvent &e) {
  mouseEvent( e.x/float(w()), e.y/float(h()), e.button, MiniMapView::Down );
  pressed = true;
  }

void MiniMapView::mouseDragEvent(Tempest::MouseEvent &e) {
  mouseEvent( e.x/float(w()), e.y/float(h()),
              Tempest::MouseEvent::ButtonLeft,
              MiniMapView::Drag );
  }

void MiniMapView::mouseUpEvent(Tempest::MouseEvent &e) {
  mouseEvent( e.x/float(w()), e.y/float(h()),
              e.button,
              MiniMapView::Up );
  pressed = false;
  }

void MiniMapView::aceptFog(Tempest::Pixmap &p, const Tempest::Pixmap &f) {
  for( int i=0; i<p.width(); ++i )
    for( int r=0; r<p.height(); ++r ){
      int lx = i*f.width()/p.width(),
          rx = (i+1)*f.width()/p.width(),
          ly = r*f.height()/p.height(),
          ry = (r+1)*f.height()/p.height();

      if( lx<rx && ly<ry ){
        int v = 0;
        for( int ix = lx; ix<rx; ++ix )
          for( int iy = ly; iy<ry; ++iy ){
            //const Tempest::Pixmap::Pixel pix = f.at( ix, iy);
            v += f.at( ix, iy).r;
            }

        Tempest::Pixmap::Pixel pix = {0,0,0,255};//p.at(i,r);
        int dv = (rx-lx)*(ry-ly)*255;
        //pix.r = (v*pix.r)/dv;
        //pix.g = (v*pix.g)/dv;
        //pix.b = (v*pix.b)/dv;
        pix.a = 255-(v*255)/dv;

        p.set(i,r, pix);
        }
      }
  }

void MiniMapView::drawUnits( Tempest::Pixmap & renderTo, World & wx ) {
  GraphicsSystem::Frustum f;
  GraphicsSystem::mkFrustum( wx.camera, f );


  int tw = wx.terrain().width(),
      th = wx.terrain().height();

  /*
  {

    Tempest::Pixmap::Pixel pix, pix2;
    pix.r = 0;
    pix.g = 0;
    pix.b = 0;
    pix.a = 0;
    pix.a = 255;

    pix2 = pix;
    pix2.r = 255;

    for( int i=0; i<tw; ++i )
      for( int r=0; r<th; ++r ){
        int pi = i*renderTo.width()/wx.terrain().width(),
            pr = r*renderTo.height()/wx.terrain().height();

        if( GraphicsSystem::isVisible( World::coordCast(i*Terrain::quadSize),
                                       World::coordCast(r*Terrain::quadSize),
                                       0,0, f) )
          renderTo.set(pi, pr, pix); else
          renderTo.set(pi, pr, pix2);
        }
    return;
    }*/


  Tempest::Pixmap::Pixel pix;
  pix.r = 0;
  pix.g = 0;
  pix.b = 0;
  pix.a = 255;

  const std::vector<World::PGameObject> & objects = wx.activeObjects();
  for( size_t i=0; i<objects.size(); ++i ){
    GameObject &obj = *objects[i];
    int terrX = (obj.x()+Terrain::quadSize)/Terrain::quadSize;
    int terrY = (obj.y()+Terrain::quadSize)/Terrain::quadSize;

    terrX = (terrX*units.width()) /tw;
    terrY = (terrY*units.height())/th;

    pix.r = obj.teamColor().r()*255;
    pix.g = obj.teamColor().g()*255;
    pix.b = obj.teamColor().b()*255;

    if( terrX >=0 && terrX<renderTo.width() &&
        terrY >=0 && terrY<renderTo.height() )

      if( !obj.getClass().data.isBackground ){
        int sz = obj.getClass().data.size;

        int terrX0 = (obj.x())/Terrain::quadSize - sz/2;
        int terrY0 = (obj.y())/Terrain::quadSize - sz/2;
        terrX0 = (terrX0*renderTo.width()) /tw;
        terrY0 = (terrY0*renderTo.height())/th;

        int terrX1 = (obj.x())/Terrain::quadSize + sz-sz/2;
        int terrY1 = (obj.y())/Terrain::quadSize + sz-sz/2;
        terrX1 = (terrX1*renderTo.width()) /tw;
        terrY1 = (terrY1*renderTo.height())/th;

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

  int dx[] = {1, -1, 0,  0, 1,  1, -1, -1 };
  int dy[] = {0,  0, 1, -1, 1, -1,  1, -1 };
  pix.r = 0;
  pix.g = 0;
  pix.b = 0;
  pix.a = 0;
  pix.a = 254;

  for( int i=1; i+1<units.width(); ++i )
    for( int r=1; r+1<units.height(); ++r ){
      if( renderTo.at(i,r).a==0 ){
        bool ok = false;
        for( int q=0; !ok && q<8; ++q )
          if( renderTo.at(i+dx[q],r+dy[q]).a==255 )
            ok = true;

        if( ok )
          renderTo.set(i,r, pix);
        }
      }

  for( int i=0; i<renderTo.width(); ++i )
    for( int r=0; r<renderTo.height(); ++r )
      if( renderTo.at(i,r).a==0 )
        ;//renderTo.set( i, r, terr.at(i,r) );
  }

void MiniMapView::paintEvent(Tempest::PaintEvent &e) {
  Tempest::Painter p(e);
  Tempest::Bind::UserTexture u;

  u.data.nonPool = &terr;

  p.setBlendMode( Tempest::alphaBlend );
  p.setTexture( u );
  p.drawRect( 0, 0, w(), h(),
              0, 0, terr.width(), terr.height() );

  u.data.nonPool = &units;

  p.setTexture( u );
  p.drawRect( 0, 0, w(), h(),
              0, 0, units.width(), units.height() );

  u.data.nonPool = &fog;
  p.setTexture( u );
  p.drawRect( 0, 0, w(), h(),
              0, 0, fog.width(), fog.height() );

  u.data.nonPool = &hud;
  p.setTexture( u );
  p.drawRect( 0, 0, w(), h(),
              0, 0, fog.width(), fog.height() );
  }
