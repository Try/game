#include "world.h"

#include "prototypesloader.h"

#include <cmath>
#include <iostream>

#include "util/math.h"

#include <MyWidget/Painter>
#include "behavior/behaviormsgqueue.h"
#include "game.h"
#include "algo/algo.h"

#include "util/weakworldptr.h"
#include "behavior/buildingbehavior.h"

#include "util/gameserializer.h"

World::World( GraphicsSystem& g,
              Resource & r,
              PrototypesLoader &p,
              Game & gm,
              int w, int h )
  : game(gm), physics(w,h),
    spatialId(w,h),
    graphics(g),
    resource(r), prototypes(p),
    particles(scene, p, r) {
  tx = ty = 0;
  mouseObject = 0;

  terr.reset( new Terrain( w,h, r, scene, *this, p) );

  particles.setupMaterial.bind( setupMaterial );

  initTerrain();
  createTestMap();

  scene.lights().direction().resize(1);

  MyGL::DirectionLight light;
  light.setDirection( -2, -1, -2.0 );
  light.setColor    ( MyGL::Color( 0.7, 0.7, 0.7 ) );
  light.setAblimient( MyGL::Color( 0.33, 0.33,  0.35) );
  scene.lights().direction()[0] = light;


  //MyGL::DirectionLight light;
  light.setDirection( -2, 1, -2.0 );
  light.setColor    ( MyGL::Color( 0.7, 0.7, 0.7 ) );
  light.setAblimient( MyGL::Color( 0.23, 0.23,  0.35) );
  //scene.lights().direction()[0] = light;

  }

void World::emitHudAnim( const std::string &s,
                         float x, float y, float z ) {
  HudAnim *a = new HudAnim( scene, *this, prototypes.get(s),
                            prototypes,
                            resource, physics );

  a->setPosition( x, y, z+zAt(x,y) );

  hudAnims.push_back( std::unique_ptr<HudAnim>(a) );
  }

float World::zAt(float x, float y) const {
  float cx = World::coordCastD( x )/Terrain::quadSizef;
  float cy = World::coordCastD( y )/Terrain::quadSizef;

  float cz = std::max( World::coordCast( terr->heightAt(cx,cy) ),
                       World::coordCast( terr->atF(cx,cy) ) );
  cz = std::max(cz,0.0f);

  return cz;
  }

const World::CameraViewBounds &World::cameraBounds() const {
  return cameraVBounds;
  }

void World::setCameraBounds( const CameraViewBounds &c ) {
  cameraVBounds = c;
  }

void World::createTestMap() {
  return;

  int s = terr->width() * Terrain::quadSize;
/*
  for( int q=0; q<5; ++q )
    for( int i=0; i<5; ++i ){
      GameObject& obj = addObjectEnv( "worker" );
      obj.setPosition( 4000+i*1000, 4000+(q)*1000, 100 );
      }
      */
/*
  for( int i=0; i*500<s; ++i ){
    addObject( "tree0" ).setPosition( 0, i*500, 1 );
    addObject( "tree0" ).setPosition( s, i*500, 1 );

    addObject( "tree0" ).setPosition( i*500, 0, 1 );
    addObject( "tree0" ).setPosition( i*500, s, 1 );
    }*/

  createResp( 1, 2000, 2000, -1, -1 );
  createResp( 2, s-2000, 2000, 1, -1 );
  createResp( 3, 2000, s-2000, -1, 1 );
  createResp( 4, s-2000, s-2000, 1, 1 );


  addObject("pikeman", 2 ).setPosition( 15000, 2500, 1 );
  }

void World::createResp(int pl, int x, int y, int minX, int minY) {
  int wcount = 0;
  addObject("castle", pl ).setPosition( x, y, 1 );

  for( int i=1; i<5; ++i ){
    addObject("crystal" ).setPosition( x - (1100 - i*300)*minX,
                                       y -  1100*minY, 1 );

    addObject("crystal" ).setPosition( x -  1100*minX,
                                       y - (1100 - i*300)*minY, 1 );

    if( wcount<6 ){
      addObject("worker", pl ).setPosition( x - (800 - i*300)*minX,
                                         y -  800*minY, 1 );
      ++wcount;
      }

    if( wcount<6 ){
      addObject("worker", pl ).setPosition( x -  800*minX,
                                            y - (800 - i*300)*minY, 1 );
      ++wcount;
      }
    }
  }

double World::coordCast(int icoord) {
  return icoord/4000.f;
  }

double World::coordCastP(double icoord) {
  return icoord/4000.f;
  }

int World::coordCastD(double dcoord) {
  return int(dcoord*4000.0);
  }

int World::coordCastD(int dcoord) {
  return dcoord*4000;
  }

void World::moveCamera(double x, double y) {
  double a = -M_PI*camera.spinX()/180.0;

  double k = camera.distance()/3.0;
  x *= k;
  y *= k;

  camera.setPosition( camera.x() + x*cos(-a) + y*sin(-a),
                      camera.y() + x*sin( a) + y*cos( a),
                      camera.z() );

  camera.setPosition( std::min( terr->viewWidth(),
                                std::max(0.0, camera.x() )),
                      std::min( terr->viewHeight(),
                                std::max(0.0, camera.y() )),
                      0);
                      //terr.heightAt((float)camera.x(), (float)camera.y()));
                      //camera.z() );
  float cx = World::coordCastD( camera.x() )/Terrain::quadSizef;
  float cy = World::coordCastD( camera.y() )/Terrain::quadSizef;

  float  z = std::max( World::coordCast( terr->heightAt(cx,cy) ),
                       World::coordCast( terr->atF(cx,cy) ) );
  z = std::max(z,0.0f);

  camera.setPosition( camera.x(),
                      camera.y(),
                      z );
  }

GameObject &World::addObject( const std::string &proto, int pl ) {
  return addObject( prototypes.get( proto ), pl, false );
  }

GameObject &World::addObjectEnv(const std::string &proto) {
  return addObject( prototypes.get( proto ), 0, true );
  }

GameObject &World::addObject( const ProtoObject &p,
                              int pl, bool env ) {
  GameObject *obj = new GameObject( scene,
                                    *this,
                                    p, prototypes );
  obj->loadView( resource, physics, env );
  obj->setPosition(0,0,0);

  PGameObject pobj = PGameObject(obj);
  if( env ){
    eviObjects.push_back( pobj );
    } else {
    gameObjects.push_back( pobj );
    }

  if( !env ){
    if( p.data.isBackground ){
      obj->setPlayer( 0 );
      } else {
      obj->setPlayer( pl );
      nonBackground.push_back( pobj );
      }
    }

  return *obj;
  }

void World::deleteObject(GameObject *obj) {
  // TOFIX: mgsq index
  for( size_t i=0; i<game.plCount(); ++i )
    if( game.player(i).editObj==obj )
      game.player(i).editObj = 0;

  if( obj==mouseObject )
    mouseObject = 0;

  //deleteObject(  gameObjects,  obj );
  deleteObject(   eviObjects,  obj );
  deleteObject( nonBackground, obj );

  for( size_t i=0; i<wptrs.size(); ++i )
    if( wptrs[i]->v.get()==obj )
      wptrs[i]->v.reset();

  for( size_t i=0; i<gameObjects.size(); ++i ){
    if( gameObjects[i].get()==obj ){
      game.onUnitRemove(i);

      for( size_t r=i; r+1<gameObjects.size(); ++r ){
        gameObjects[r] = gameObjects[r+1];
        }

      gameObjects.pop_back();
      return;
      }
    }
  }

void World::deleteObject(std::vector< PGameObject > &c, GameObject *obj) {
  for( size_t i=0; i<c.size(); ++i )
    if( c[i].get()==obj ){
      std::swap( c[i], c.back() );
      c.pop_back();
      }
  }

size_t World::objectsCount() const {
  return gameObjects.size();
  }

GameObject &World::object(size_t i) {
  return *gameObjects[i];
  }

const GameObject &World::object(size_t i) const {
  return *gameObjects[i];
  }

WeakWorldPtr World::objectWPtr(size_t i) {
  return WeakWorldPtr( *this, gameObjects[i] );
  }

WeakWorldPtr World::objectWPtr(GameObject *x) {
  if( &x->world() != this)
    return WeakWorldPtr(); else{
    for( size_t i=0; i<gameObjects.size(); ++i )
      if( gameObjects[i].get()==x )
        return WeakWorldPtr( *this, gameObjects[i] );
    }

  return WeakWorldPtr();
  }

const Terrain &World::terrain() const {
  return *terr;
  }

Terrain &World::terrain() {
  return *terr;
  }

void World::updateMouseOverFlag( double x0, double y0,
                                 double x1, double y1 ) {
  MyGL::Matrix4x4 mat = camera.projective();
  mat.mul( camera.view() );
  //mat.transpose();

  if( x0 > x1 )
    std::swap(x0,x1);

  if( y0 > y1 )
    std::swap(y0,y1);

  double data[4];

  bool hostCtrl = 0,
       hostUnit = 0;

  for( size_t i=0; i<gameObjects.size(); ++i ){
    GameObject & obj = *gameObjects[i];
    double x = coordCast( obj.x() ),
           y = coordCast( obj.y() ),
           z = coordCast( obj.z() );

    mat.project( x, y, z, 1,
                 data[0], data[1], data[2], data[3] );
    for( int i=0; i<3; ++i )
      data[i] /= data[3];

    obj.setMouseOverFlag( x0 <= data[0] && data[0] <= x1 &&
                          y0 <= data[1] && data[1] <= y1 );

    if( obj.isMouseOwer() && obj.hasHostCtrl() ){
      hostCtrl = 1;
      if( obj.behavior.find<BuildingBehavior>()==0 )
        hostUnit = 1;
      }
    }

  if( hostCtrl ){
    for( size_t i=0; i<gameObjects.size(); ++i ){
      GameObject & obj = *gameObjects[i];
      if( !obj.hasHostCtrl() ||
          (hostUnit && obj.behavior.find<BuildingBehavior>()!=0))
        obj.setMouseOverFlag(0);
      }
    }
  }

void World::updateSelectionFlag( BehaviorMSGQueue & msg, int pl ) {
  size_t ibegin = -1, isize = 0;

  for( size_t i=0; i<gameObjects.size(); ++i ){
    GameObject & obj = *gameObjects[i];

    if( obj.isMouseOwer() && obj.hasHostCtrl() ){
      if( ibegin==size_t(-1) || (ibegin+isize != i) ){
        if( ibegin!=size_t(-1) )
          msg.message_st( pl, Behavior::SelectAdd, ibegin, isize );
        if( ibegin==size_t(-1) )
          msg.message( pl, Behavior::UnSelect );

        ibegin = i;
        isize  = 1;
        } else {
        ++isize;
        }
      }

    obj.setMouseOverFlag(0);
    }

  if( ibegin!=size_t(-1) ){
    msg.message_st( pl, Behavior::SelectAdd, ibegin, isize );
    }
  }

void World::updateSelectionClick( BehaviorMSGQueue &msg,
                                  int pl,
                                  int mx, int my,
                                  int w, int h ) {
  size_t i = unitUnderMouse(mx,my,w,h);
  if( i==size_t(-1) )
    return;

  msg.message( pl, Behavior::UnSelect );
  msg.message_st( pl, Behavior::SelectAdd, i, 1 );
  }

size_t World::unitUnderMouse( int mx, int my, int w, int h ) const {
  MyGL::Matrix4x4 gmMat = camera.projective();
  gmMat.mul( camera.view() );

  int dist = -1, mdist = -1;
  size_t ret = -1;

  for( size_t i=0; i<objectsCount(); ++i ){
    if( object(i).getClass().data.isBackground )
      continue;

    if( isUnitUnderMouse( gmMat, object(i), mx, my, w, h, dist ) ){
      //return i;
      if( mdist<0 || dist<mdist ){
        ret = i;
        mdist = dist;
        }
      }
    }

  if( mdist>=0 ){
    return ret;
    }

  for( size_t i=0; i<resouces.size(); ++i ){
    if( isUnitUnderMouse( gmMat, *resouces[i], mx, my, w, h, dist ) ){
      for( size_t r=0; r<objectsCount(); ++r )
        if( &object(r)==resouces[i] )
          return r;
      }
    }

  return -1;
  }



bool World::isUnitUnderMouse( MyGL::Matrix4x4 & gmMat,
                              const GameObject & obj,
                              int mx, int my, int w, int h,
                              int & dist ) const {
  double data1[4], data2[4];
  MyGL::Matrix4x4 m = gmMat;

  m.mul( obj._transform() );

  MyGL::Matrix4x4 mat = obj._transform();

  double left[4] = { mat.data()[0], mat.data()[4], mat.data()[8], 0 };
  double  top[4] = { mat.data()[1], mat.data()[5], mat.data()[9], 0 };

  for( int r=0; r<3; ++r ){
    left[3] += left[r]*left[r];
    top [3] += top [r]*top [r];
    }
  left[3] = sqrt(left[3]);
  top [3] = sqrt( top[3]);

  for( int r=0; r<3; ++r ){
    left[r] /= left[3];
    top [r] /=  top[3];

    left[r] += top[r];
    }

  double x = 0,
         y = 0,
         z = 0;//0.5*obj.rawRadius();

  double r = 0.8*obj.rawRadius();

  m.project( x+r*left[0], y+r*left[1], z+r*left[2], 1,
             data1[0], data1[1], data1[2], data1[3] );
  for( int r=0; r<3; ++r )
    data1[r] /= data1[3];

  m.project( x-r*left[0], y-r*left[1], z-r*left[2], 1,
             data2[0], data2[1], data2[2], data2[3] );
  for( int r=0; r<3; ++r )
    data2[r] /= data2[3];

  data1[0] = 0.5*(1+data1[0])*w;
  data1[1] = 0.5*(1-data1[1])*h;

  data2[0] = 0.5*(1+data2[0])*w;
  data2[1] = 0.5*(1-data2[1])*h;

  if( data2[0] <= mx && mx <= data1[0] &&
      data2[1] <= my && my <= data1[1] ){
    int midX = (data1[0]+data2[0])/2;
    int midY = (data1[1]+data2[1])/2;

    dist = std::max( abs(midX-mx), abs(midY-my) );
    return true;
    }

  dist = -1;
  return false;
  }

void World::paintHUD( MyWidget::Painter & p,
                      int w, int h ) {
  //return;

  MyGL::Matrix4x4 gmMat = camera.projective();
  gmMat.mul( camera.view() );

  MyWidget::Bind::UserTexture green, bar, gray;
  green.data = resource.pixmap("gui/hp");
  bar.data   = resource.pixmap("gui/bar");
  gray.data  = resource.pixmap("gui/gray");

  double data1[4], data2[4];

  for( size_t plN = 1; plN<game.plCount(); ++plN )
    for( size_t i=0; i<game.player(plN).unitsCount(); ++i ){
      MyGL::Matrix4x4 m = gmMat;

      GameObject & obj = game.player(plN).unit(i);
      m.mul( obj._transform() );
      //m.transpose();

      MyGL::Matrix4x4 mat = obj._transform();

      double left[4] = { mat.data()[0], mat.data()[4], mat.data()[8], 0 };
      double  top[4] = { mat.data()[1], mat.data()[5], mat.data()[9], 0 };

      for( int r=0; r<3; ++r ){
        left[3] += left[r]*left[r];
        top [3] += top [r]*top [r];
        }
      left[3] = sqrt(left[3]);
      top [3] = sqrt( top[3]);

      for( int r=0; r<3; ++r ){
        left[r] /= left[3];
        top [r] /=  top[3];

        left[r] += top[r];
        }

      double x = 0,//obj.x(),
             y = 0,//obj.y(),
             z = 0.5*obj.rawRadius();//obj.z();

      double r = 0.5*obj.rawRadius();

      m.project( x+r*left[0], y+r*left[1], z+r*left[2], 1,
                 data1[0], data1[1], data1[2], data1[3] );
      for( int i=0; i<3; ++i )
        data1[i] /= data1[3];

      m.project( x-r*left[0], y-r*left[1], z-r*left[2], 1,
                 data2[0], data2[1], data2[2], data2[3] );
      for( int i=0; i<3; ++i )
        data2[i] /= data2[3];

      int y0 = 0.5*(1-data2[1])*h;
      int x0 = 0.5*(1+data2[0])*w;
      int x1 = 0.5*(1+data1[0])*w;

      int sz = ( int(20*obj.getClass().data.size)/4 )*4+1;

      int bx = (sz*obj.hp())/obj.getClass().data.maxHp;

      p.setTexture( green );
      p.drawRect( (x0+x1-sz)/2, y0, bx, 5,
                  0,0, 65, 5 );

      p.setTexture( gray );
      p.drawRect( (x0+x1-sz)/2+bx, y0, sz-bx, 5,
                  0,0, 65, 5 );

      p.setTexture( bar );
      p.drawRectTailed( (x0+x1-sz)/2, y0, sz, 5,
                        0,0, 65, 5 );
      p.drawRect( (x0+x1-sz)/2+sz-1, y0, 1, 5,
                  64,0, 1, 5 );

      //p.drawRect( 0.5*(1+data1[0])*w, 0.5*(1-data1[1])*h, 10, 10 );
      //p.drawRect( 0.5*(1+data2[0])*w, 0.5*(1-data2[1])*h, 10, 10 );
      }

  }

Player &World::player(int id) {
  return game.player(id);
  }

void World::addWptr( WeakWorldPtr *p ) {
  wptrs.push_back( p );
  }

void World::delWptr( WeakWorldPtr *p ) {
  for( size_t i=0; i<wptrs.size(); ++i )
    if( p==wptrs[i] ){
      std::swap( wptrs[i], wptrs.back() );
      wptrs.pop_back();
      return;
      }
  }

void World::addWarehouse (GameObject *ptr) {
  warehouses.push_back( ptr );
  }

void World::delWarehouse (GameObject *p) {
  del( warehouses, p );
  }

const std::vector<GameObject*> &World::warehouse() const {
  return warehouses;
  }

void World::del( std::vector<GameObject*> & x, GameObject *p) {
  for( size_t i=0; i<x.size(); ++i )
    if( p==x[i] ){
      std::swap( x[i], x.back() );
      x.pop_back();
      return;
      };
  }

void World::addResouce(GameObject *ptr) {
  resouces.push_back( ptr );
  }

void World::delResouce (GameObject *p) {
  del( resouces, p );
  }

const std::vector<GameObject*> &World::resouce() const {
  return resouces;
  }

void World::setMousePos(int x, int y, int z) {
  mpos[0] = x;
  mpos[1] = y;
  mpos[2] = z;
  }

void World::setMouseObject(size_t i) {
  if( mouseObject )
    mouseObject->setMouseOverFlag(0);

  if( i==size_t(-1) ){
    mouseObject = 0;
    } else {
    mouseObject = gameObjects[i].get();
    mouseObject->setMouseOverFlag(1);
    }
  }

int World::mouseX() const {
  return mpos[0];
  }

int World::mouseY() const {
  return mpos[1];
  }

int World::mouseZ() const {
  return mpos[2];
  }

GameObject * World::mouseObj() {
  return mouseObject;
  }

void World::toogleEditLandMode(const Terrain::EditMode &m) {
  editLandMode = m;
  }

void World::initTerrain() {
  terr->buildGeometry( graphics.lvboHolder,
                       graphics.iboHolder );
  physics.setTerrain( terrain() );
  }

void World::clickEvent(int x, int y, const MyWidget::MouseEvent &e) {
  if( e.button==MyWidget::MouseEvent::ButtonLeft ){
    tx = x;
    ty = y;
    }

  if( editLandMode.isEnable ){
    if( e.button==MyWidget::MouseEvent::ButtonRight )
      terrain().brushHeight( x, y, editLandMode, true );//-200, 5 );

    if( e.button==MyWidget::MouseEvent::ButtonLeft )
      terrain().brushHeight( x, y, editLandMode, false );//200, 5 );

    for( size_t i=0; i<objectsCount(); ++i ){
      GameObject & obj = object(i);
      float wx = obj.x()/Terrain::quadSizef,
            wy = obj.y()/Terrain::quadSizef;

      obj.setPosition( obj.x(), obj.y(), terrain().heightAt(wx,wy) );
      }

    terr->buildGeometry( graphics.vboHolder,
                         graphics.iboHolder );
    physics.setTerrain( *terr );
    }
  }

void World::onRender() {
  scene.setCamera( camera );
  terr->updatePolish();
  }

void World::tick() {
  //return;

  spatialId.fill( nonBackground );
  spatialId.solveColisions();

  for( size_t i=0; i<nonBackground.size(); ++i ){
    GameObject & obj = *nonBackground[i];
    obj.tick( terrain() );
    }

  for( size_t i=0; i<resouces.size(); ++i ){
    GameObject & obj = *resouces[i];
    if( obj.getClass().data.isBackground )
      obj.tick( terrain() );
    }

  spatialId.clear();

  physics.tick();

  for( size_t i=0; i<hudAnims.size(); ++i )
    hudAnims[i]->tick();

  for( size_t i=0; i<hudAnims.size();  ){
    if( hudAnims[i]->isEnd() ){
      std::swap( hudAnims[i], hudAnims.back() );
      hudAnims.pop_back();
      } else {
      ++i;
      }
    }

  for( size_t i=0; i<nonBackground.size(); ++i )
    if( nonBackground[i]->hp() <= 0 ) {
      GameObject & src = *nonBackground[i];
      GameObject & obj = addObjectEnv( nonBackground[i]->getClass().name );

      obj.setTeamColor( src.teamColor() );
      MyGL::Color cl = obj.teamColor();
      float k = 0.7;
      cl.set( cl.r()*k, cl.g()*k, cl.b()*k, cl.a() );
      obj.setTeamColor( cl );

      obj.setPosition( src.x(), src.y(), src.z()+100 );
      //obj.rotate();
      }

  for( size_t i=0; i<nonBackground.size(); ){
    if( nonBackground[i]->hp() <= 0 ) {
      deleteObject( nonBackground[i].get() );
      } else {
      ++i;
      }
    }

  for( size_t i=0; i<gameObjects.size(); ++i )
    gameObjects[i]->updatePos();

  for( size_t i=0; i<eviObjects.size(); ++i )
    eviObjects[i]->updatePos();
  }

const MyGL::Scene &World::getScene() const {
  return scene;
  }

ParticleSystemEngine &World::getParticles() {
  return particles;
  }

void World::serialize(GameSerializer &s) {
  terr->serialize(s);
  spatialId = SpatialIndex( terr->width(),
                            terr->height() );

  if( s.isReader() ){
    terr->buildGeometry( graphics.vboHolder,
                         graphics.iboHolder );

    physics.setTerrain( *terr );
    }

  unsigned sz = gameObjects.size();
  s + sz;

  if( s.isReader() ){
    gameObjects.clear();
    eviObjects.clear();
    warehouses.clear();
    resouces.clear();
    wptrs.clear();
    }

  for( size_t i=0; i<sz; ++i ){
    if( s.isReader() ){
      std::string str;
      s + str;

      addObject( str );
      gameObjects[i]->serialize(s);
      } else {
      std::string str = gameObjects[i]->getClass().name;
      s + str;
      gameObjects[i]->serialize(s);
      }

    }
  }

const SpatialIndex &World::spatial() const {
  return spatialId;
  }
