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

World::World( GraphicsSystem& g,
              Resource & r,
              PrototypesLoader &p,
              Game & gm,
              int w, int h )
  : game(gm), physics(w,h), terr(w,h), graphics(g), resource(r), prototypes(p) {
  tx = ty = 0;

  initTerrain();
  createTestMap();

  scene.lights().direction().resize(1);
  MyGL::DirectionLight light;
  light.setDirection( -2, -1, -2.0 );
  light.setColor    ( MyGL::Color( 0.7, 0.7, 0.7 ) );
  light.setAblimient( MyGL::Color( 0.33, 0.33,  0.35) );
  scene.lights().direction()[0] = light;
  }

void World::createTestMap() {
  int s = terr.width() * Terrain::quadSize;

  for( int q=0; q<5; ++q )
    for( int i=0; i<5; ++i ){
      GameObject& obj = addObjectEnv( "worker" );
      obj.setPosition( 4000+i*1000, 4000+(q)*1000, 100 );
      }

  for( int i=0; i*500<s; ++i ){
    addObject( "tree0" ).setPosition( 0, i*500, 1 );
    addObject( "tree0" ).setPosition( s, i*500, 1 );

    addObject( "tree0" ).setPosition( i*500, 0, 1 );
    addObject( "tree0" ).setPosition( i*500, s, 1 );
    }

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
  return icoord*0.001;
  }

double World::coordCastP(double icoord) {
  return icoord*0.001;
  }

int World::coordCastD(double dcoord) {
  return int(dcoord*1000.0);
  }

int World::coordCastD(int dcoord) {
  return dcoord*1000;
  }

void World::moveCamera(double x, double y) {
  double a = -M_PI*camera.spinX()/180.0;

  double k = camera.distance()/3.0;
  x *= k;
  y *= k;

  camera.setPosition( camera.x() + x*cos(-a) + y*sin(-a),
                      camera.y() + x*sin( a) + y*cos( a),
                      camera.z() );

  camera.setPosition( std::min( terr.viewWidth(),
                                std::max(0.0, camera.x() )),
                      std::min( terr.viewWidth(),
                                std::max(0.0, camera.y() )),
                      camera.z() );
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

  if( env )
    eviObjects.push_back( PGameObject(obj) );   else
    gameObjects.push_back( PGameObject( obj ) );

  if( !env )
    obj->setPlayer( pl );

  return *obj;
  }

void World::deleteObject(GameObject *obj) {
  for( size_t i=0; i<game.plCount(); ++i )
    if( game.player(i).editObj==gameObjects[i].get() )
      game.player(i).editObj = 0;

  deleteObject( gameObjects, obj );
  deleteObject(  eviObjects, obj );
  //deleteObject(    selected, obj );

  for( size_t i=0; i<wptrs.size(); ++i )
    if( wptrs[i]->v.get()==obj )
      wptrs[i]->v.reset();
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
  return terr;
  }

Terrain &World::terrain() {
  return terr;
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
    }
  }

void World::updateSelectionFlag( BehaviorMSGQueue & msg, int pl ) {
  msg.message( pl, Behavior::UnSelect );

  size_t ibegin = -1, isize = 0;

  for( size_t i=0; i<gameObjects.size(); ++i ){
    GameObject & obj = *gameObjects[i];

    if( obj.isMouseOwer() && obj.hasHostCtrl() ){
      if( ibegin==size_t(-1) || (ibegin+isize != i) ){
        if( ibegin!=size_t(-1) )
          msg.message_st( pl, Behavior::SelectAdd, ibegin, isize );
        ibegin = i;
        isize  = 1;
        } else {
        ++isize;
        }
      }
    }

  if( ibegin!=size_t(-1) ){
    msg.message_st( pl, Behavior::SelectAdd, ibegin, isize );
    }
  }

void World::paintHUD( MyWidget::Painter & p,
                      int w, int h ) {
  return;

  MyGL::Matrix4x4 mat;// = camera.projective();
  mat.mul( camera.view() );

  double data1[4], data2[4];

  for( size_t i=0; i<1; ++i ){
    MyGL::Matrix4x4 m = mat;

    GameObject & obj = *terrainView;
    m.mul( obj._transform() );
    //m.transpose();

    double left[4] = { m.at(0,0), m.at(1,0), m.at(2,0), 0 };
    double  top[4] = { m.at(0,1), m.at(1,1), m.at(2,1), 0 };

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

    double x = 9.60000038,
           y = 9.60000038,
           z = -0.449999988;

    double r = obj.rawRadius();

    m.project( x+r*left[0], y+r*left[1], z+r*left[2], 1,
               data1[0], data1[1], data1[2], data1[3] );
    for( int i=0; i<3; ++i )
      data1[i] /= data1[3];

    m.project( x-r*left[0], y-r*left[1], z-r*left[2], 1,
               data2[0], data2[1], data2[2], data2[3] );
    for( int i=0; i<3; ++i )
      data2[i] /= data2[3];


    p.drawRect( 0.5*(1+data1[0])*w, 0.5*(1-data1[1])*h, 10, 10 );
    p.drawRect( 0.5*(1+data2[0])*w, 0.5*(1-data2[1])*h, 10, 10 );

    if(!(data2[0]<data1[0] /*&& data2[1]<data1[1]*/ )){
      std::cout << "FBF";
      }
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

void World::setMousePos(int x, int y) {
  mpos[0] = x;
  mpos[1] = y;
  }

int World::mouseX() const {
  return mpos[0];
  }

int World::mouseY() const {
  return mpos[1];
  }

void World::initTerrain() {
  GameObject *obj = new GameObject( scene,
                                    *this,
                                    prototypes.get("land"),
                                    prototypes );
  obj->setPosition(0,0,0);
  obj->loadView( terr.buildGeometry( graphics.vboHolder,
                                     graphics.iboHolder ) );

  physics.setTerrain( terrain() );
  terrainView =  PGameObject(obj);

  obj = new GameObject( scene,
                        *this,
                        prototypes.get("water"),
                        prototypes );
  obj->setPosition(0,0,0);

  obj->loadView( terr.waterGeometry( graphics.vboHolder,
                                     graphics.iboHolder ) );
  waterView  = PGameObject(obj);
  }

void World::clickEvent(int x, int y, const MyWidget::MouseEvent &e) {
  if( e.button==MyWidget::MouseEvent::ButtonLeft ){
    tx = x;
    ty = y;
    }

  if( e.button==MyWidget::MouseEvent::ButtonRight ){
    //terrain().brushHeight( x, y, -200, 5 );

    terrainView->loadView( terr.buildGeometry( graphics.vboHolder,
                                                  graphics.iboHolder ) );
    waterView->loadView  ( terr.waterGeometry( graphics.vboHolder,
                                                  graphics.iboHolder ) );
    physics.setTerrain( terr );
    }
  }

void World::tick() {
  scene.setCamera( camera );

  terr.resetBusyMap();
  for( size_t i=0; i<gameObjects.size(); ++i ){
    GameObject & obj = *gameObjects[ gameObjects.size()-i-1 ];
    int wx = obj.x()/Terrain::quadSize,
        wy = obj.y()/Terrain::quadSize;

    terr.incBusyAt(wx,wy, obj);
    }

  for( size_t i=0; i<gameObjects.size(); ++i ){
    GameObject & obj = *gameObjects[i];
    obj.tick( terrain() );
    }

  physics.tick();

  for( size_t i=0; i<gameObjects.size(); ++i )
    if( gameObjects[i]->hp() <= 0 ) {
      GameObject & src = *gameObjects[i];
      GameObject & obj = addObjectEnv( gameObjects[i]->getClass().name );

      obj.setPlayer( src.playerNum() );
      obj.setPosition( src.x(), src.y(), src.z()+100 );
      //obj.rotate();
      }

  for( size_t i=0; i<gameObjects.size(); ){
    if( gameObjects[i]->hp() <= 0 ) {
      deleteObject( gameObjects[i].get() );
      } else {
      ++i;
      }
    }

  for( size_t i=0; i<gameObjects.size(); ++i )
    gameObjects[i]->updatePos();

  for( size_t i=0; i<eviObjects.size(); ++i )
    eviObjects[i]->updatePos();

  // FIXME
  /*
  selected.clear();
  for( size_t i=0; i<gameObjects.size(); ++i )
    if( gameObjects[i]->isSelected() )
      selected.push_back( gameObjects[i] );
      */
  }

const MyGL::Scene &World::getScene() const {
  return scene;
  }

