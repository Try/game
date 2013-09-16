#include "minimapview.h"

#include "resource.h"

#include "game/world.h"
#include "game.h"

MiniMapView::MiniMapView( Resource &res ):TextureView(res), res(res) {
  rtime   = 0;
  rtime2  = 0;
  tcount  = -1;

  pressed = false;
  world   = 0;

  needToUpdateTerrain  = true;
  needToUpdateTerrainV = true;
  }

MiniMapView::~MiniMapView() {
  if( world )
    world->terrain().onTerrainChanged.ubind( this, &MiniMapView::onTerrainCanged );
  }

void MiniMapView::setup(World * w) {
  tcount  = -1;
  rtime   = 0;
  rtime2  = 0;

  if( world )
    world->terrain().onTerrainChanged.ubind( this, &MiniMapView::onTerrainCanged );
  world = w;
  needToUpdateTerrain = true;
  needToUpdateTerrainV = true;

  if(w)
    w->terrain().onTerrainChanged.bind( this, &MiniMapView::onTerrainCanged );
  }

void MiniMapView::render() {
  if( world==0 )
    return;

  if( world->game.isPaused() )
    return;

  World &wx = *world;

  int mk = std::max( 1, wx.terrain().width()*wx.terrain().height()/(128*128) );

  if( terr.width() != w() || terr.height()!= h() ){
    rtime = tcount - mk*200;
    tmpPix = Tempest::Pixmap(w(), h(), true);

    update();
    }

  if( needToUpdateTerrainV && rtime2+mk*50 < tcount ){
    needToUpdateTerrainV = false;
    rtime2 = tcount;

    Tempest::Pixmap& terr = tmpPix;
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

  if( //needToUpdateTerrain ||
      tcount >= rtime+mk*40 ||
      tcount < rtime ){
    rtime = tcount;

    Tempest::Pixmap::Pixel pix = {0,0,0,0};
    Tempest::Pixmap& renderTo = tmpPix;
    renderTo.fill(pix);

    drawUnits(renderTo, wx);
    //aceptFog(renderTo, wx.game.player().fog() );
    units = res.ltexHolder.create(renderTo, false, false);

    Tempest::Pixmap& fogTex = tmpPix;
    aceptFog(fogTex, wx.game.player().fog() );

    fog   = res.ltexHolder.create(fogTex, false, false);
    }

  if( camBounds != wx.cameraBounds() ){
    camBounds = wx.cameraBounds();
    int sx = wx.terrain().width()*Terrain::quadSize,
        sy = wx.terrain().height()*Terrain::quadSize;

    for( int r=0; r<4; ++r ){
      camBounds.x[r] = (camBounds.x[r]*w())/sx;
      camBounds.y[r] = (camBounds.y[r]*h())/sy;
      }

    update();
    }

  //hud   = res.ltexHolder.create(hudPx, false, false);
  needToUpdateTerrain = false;
  ++tcount;
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
  int tw = wx.terrain().width(),
      th = wx.terrain().height();

  Tempest::Pixmap::Pixel pix;
  pix.r = 0;
  pix.g = 0;
  pix.b = 0;
  pix.a = 255;

  const std::vector<World::PGameObject> & objects = wx.activeObjects();
  for( size_t i=0; i<objects.size(); ++i ){
    GameObject &obj = *objects[i];
    if( !obj.getClass().data.isBackground ){
      int ox = obj.x()/Terrain::quadSize,
          oy = obj.y()/Terrain::quadSize;

      int terrX = ox+1,
          terrY = oy+1;

      terrX = (terrX*units.width()) /tw;
      terrY = (terrY*units.height())/th;

      pix.r = obj.teamColor().r()*255;
      pix.g = obj.teamColor().g()*255;
      pix.b = obj.teamColor().b()*255;

      if( terrX >=0 && terrX<renderTo.width() &&
          terrY >=0 && terrY<renderTo.height() ){
          int sz = obj.getClass().data.size;

          int terrX0 = ox - sz/2;
          int terrY0 = oy - sz/2;
          terrX0 = (terrX0*renderTo.width()) /tw;
          terrY0 = (terrY0*renderTo.height())/th;

          int terrX1 = ox + sz-sz/2;
          int terrY1 = oy + sz-sz/2;
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
  }

void MiniMapView::onTerrainCanged() {
  needToUpdateTerrain  = true;
  needToUpdateTerrainV = true;
  //hudPx.fill( {0,0,0,0} );
  }

void MiniMapView::paintEvent(Tempest::PaintEvent &e) {
  Tempest::Painter p(e);

  p.setBlendMode( Tempest::alphaBlend );
  p.setTexture( terr );
  p.drawRect( 0, 0, w(), h(),
              0, 0, terr.width(), terr.height() );

  p.setTexture( units );
  p.drawRect( 0, 0, w(), h(),
              0, 0, units.width(), units.height() );

  p.setTexture( fog );
  p.drawRect( 0, 0, w(), h(),
              0, 0, fog.width(), fog.height() );

  /*
  p.setTexture( hud );
  p.drawRect( 0, 0, w(), h(),
              0, 0, fog.width(), fog.height() );
              */

  p.unsetTexture();
  p.drawLine( camBounds.x[0], camBounds.y[0],
              camBounds.x[1], camBounds.y[1] );
  p.drawLine( camBounds.x[1], camBounds.y[1],
              camBounds.x[3], camBounds.y[3] );
  p.drawLine( camBounds.x[2], camBounds.y[2],
              camBounds.x[3], camBounds.y[3] );
  p.drawLine( camBounds.x[0], camBounds.y[0],
              camBounds.x[2], camBounds.y[2] );
  }
