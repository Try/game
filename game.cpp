#include "game.h"

#include <MyGL/TessObject>
#include <MyWidget/Event>
#include <iostream>

#include <windows.h>
#include "util/lexicalcast.h"
#include "util/factory.h"

#include "util/serialize.h"
#include "algo/algo.h"
#include <MyGL/Pixmap>

#include "graphics/displacematerial.h"
#include "graphics/glowmaterial.h"
#include "graphics/transparentmaterial.h"
#include "graphics/addmaterial.h"
#include "graphics/watermaterial.h"
#include "graphics/mainmaterial.h"
#include "graphics/omnimaterial.h"
#include "graphics/blushmaterial.h"
#include "graphics/terrainminormaterial.h"

#include "util/gameserializer.h"

#include "behavior/buildingbehavior.h"

#include <cmath>

Game::Game( void *ihwnd, int iw, int ih, bool isFS )
  : graphics( ihwnd, iw, ih, isFS, isFS ? 2048:1024 ),
    resource( graphics.texHolder,
              graphics.localTex,
              graphics.vboHolder,
              graphics.lvboHolder,
              graphics.iboHolder,
              graphics.vsHolder,
              graphics.fsHolder ),
    gui( graphics.device, iw, ih, resource, proto ),
    serializator("./serialize_tmp.obj", Serialize::Write ){
  w = iw;
  h = ih;
  isFullScreen = isFS;

  curMPos = MyWidget::Point(w/2, h/2);

  hwnd = ihwnd;
  currentPlayer = 1;

  spinX =  0;
  spinY =  -150;
  mouseTracking         = 0;
  selectionRectTracking = 0;

  initFactorys();

  resource.load("./data/data.xml");
  proto   .load("./data/game.xml");

  //MyGL::Model<>::saveRawData( "./sphere.mx", MyGL::TessObject::sphere(3, 1) );

  setPlaylersCount(1);

  gui.createControls( msg, *this );
  gui.enableHooks( !serializator.isReader() );
  gui.toogleFullScreen.bind( *this, &Game::toogleFullScr );
  gui.addObject.bind( *this, &Game::createEditorObject );
  gui.setCameraPos.bind(*this, &Game::setCameraPos );

  gui.save.bind( *this, &Game::save );
  gui.load.bind( *this, &Game::load );

  graphics.load( resource, gui, w, h );

  gui.renderScene.bind( graphics, &GraphicsSystem::renderSubScene );

  worlds.push_back( std::unique_ptr<World>( new World(graphics, resource,
                                                      proto,
                                                      *this,
                                                      256, 256) ) );

  world = worlds[0].get();
  world->setupMaterial.bind(*this, &Game::setupMaterials );

  gui.toogleEditLandMode.bind( *world, &World::toogleEditLandMode );

  world->camera.setPerespective( true, w, h );
  world->camera.setPosition( 2, 3, 0 );
  world->camera.setDistance( 4 );

  gui.paintObjectsHud.bind( *world, &World::paintHUD );

  fps.n    = 0;
  fps.time = 0;
  }

void Game::tick() {
  moveCamera();

  DWORD time = GetTickCount();

  World::CameraViewBounds b;
  F3 vb[4];
  vb[0] = unProject( 0, 0 );
  vb[1] = unProject( w, 0 );
  vb[2] = unProject( 0, h );
  vb[3] = unProject( w, h );

  for( int i=0; i<4; ++i ){
    b.x[i] = World::coordCastD(vb[i].data[0]);
    b.y[i] = World::coordCastD(vb[i].data[1]);
    }

  world->setCameraBounds(b);

  F3 v = unProject( curMPos.x, curMPos.y );
  int vx = World::coordCastD(v.data[0])/Terrain::quadSize,
      vy = World::coordCastD(v.data[1])/Terrain::quadSize;
  world->setMousePos( vx*Terrain::quadSize,
                      vy*Terrain::quadSize,
                      World::coordCastD(v.data[2]) );

  if( player().editObj ){
    int x = World::coordCastD(v.data[0]),
        y = World::coordCastD(v.data[1]);

    if( player().editObj &&
        player().editObj->behavior.find<BuildingBehavior>() ){
      x = (x/Terrain::quadSize)*Terrain::quadSize;
      y = (y/Terrain::quadSize)*Terrain::quadSize;
      }

    msg.message( currentPlayer,
                 Behavior::EditMove,
                 x, y );
    }

  msg.serialize( serializator );
  msg.tick( *this, *world );

  world->tick();

  fps.time += int(GetTickCount() - time);
  }

void Game::render( size_t dt ) {
  world->camera.setSpinX(spinX);
  world->camera.setSpinY(spinY);

  DWORD time = GetTickCount();

  if( graphics.render( world->getScene(),
                       world->getParticles(), dt )){
    gui.renderMinimap(*world);
    world->onRender();
    }

  ++fps.n;
  fps.time += int(GetTickCount() - time);

  if( fps.n>100 || ( fps.n>0 && fps.time>1000 ) ){
    double f = 1000.0*double(fps.n)/std::max(1, fps.time);
    SetWindowTextA( HWND(hwnd),
                    Lexical::upcast( f ).data() );

    fps.n    = 0;
    fps.time = 0;
    }

  }

void Game::resizeEvent( int iw, int ih ){
  w = iw;
  h = ih;

  // std::cout << w << " " << h << std::endl;

  //world->camera.setPerespective(true, w, h );

  graphics.resizeEvent( w, h, isFullScreen );
  gui.resizeEvent(w,h);
  }

void Game::mouseDownEvent( MyWidget::MouseEvent &e) {
  if( gui.mouseDownEvent(e) )
    return;

  gui.setFocus();
  mouseTracking         = (e.button==MyWidget::MouseEvent::ButtonRight);
  if(e.button==MyWidget::MouseEvent::ButtonLeft)
    selectionRectTracking = 1;

  gui.selectionRect() = MyWidget::Rect(e.x, e.y, 0, 0);

  lastMPos = MyWidget::Point( e.x, e.y );

  if( e.button==MyWidget::MouseEvent::ButtonLeft && player().editObj ){
    msg.message( currentPlayer, Behavior::EditNext );
    }

  if( e.button==MyWidget::MouseEvent::ButtonRight && player().editObj ){
    msg.message( currentPlayer, Behavior::EditDel );
    }
  }

void Game::mouseUpEvent( MyWidget::MouseEvent &e) {
  if( gui.mouseUpEvent(e) )
    return;

  gui.setFocus();

  mouseTracking         = false;
  gui.selectionRect() = MyWidget::Rect(-1, -1, 0, 0);
  gui.update();

  F3 v = unProject( e.x, e.y );

  if( e.button==MyWidget::MouseEvent::ButtonLeft ){
    if( selectionRectTracking==2 )
      world->updateSelectionFlag( msg, currentPlayer ); else
      world->updateSelectionClick( msg, currentPlayer, e.x, e.y,
                                   w, h );
    //gui.updateSelectUnits( *world );
    }

  if( e.button==MyWidget::MouseEvent::ButtonRight ){
    msg.message( currentPlayer,
                 AbstractBehavior::Move,
                 World::coordCastD(v.data[0]),
                 World::coordCastD(v.data[1]) );
    //world->emitHudAnim( "hud/move", v.data[0], v.data[1], v.data[2]+0.01 );
    }

  selectionRectTracking = false;
  world->clickEvent( World::coordCastD(v.data[0]),
                     World::coordCastD(v.data[1]),
                     e );
  }

void Game::mouseMoveEvent( MyWidget::MouseEvent &e ) {
  if( gui.mouseMoveEvent(e) ){
    return;
    }

  curMPos = MyWidget::Point(e.x, e.y);

  if( mouseTracking ){
    //spinX -= (e.x - lastMPos.x);
    //spinY -= (e.y - lastMPos.y);

    lastMPos = MyWidget::Point(e.x, e.y);
    }

  if( selectionRectTracking ){
    selectionRectTracking = 2;

    MyWidget::Rect & r = gui.selectionRect();
    r.w = e.x - gui.selectionRect().x;
    r.h = e.y - gui.selectionRect().y;

    gui.update();

    double ww = w/2.0, hh = h/2.0;
    world->updateMouseOverFlag(    r.x/ww - 1.0,
                                 -(r.y/hh - 1.0),
                                  (r.x+r.w)/ww - 1.0,
                                -((r.y+r.h)/hh - 1.0) );
    }

  }

void Game::mouseWheelEvent( MyWidget::MouseEvent &e ) {
  if( gui.mouseWheelEvent(e) ){
    return;
    }
  gui.setFocus();

  if( (player().editObj && !serializator.isReader() ) &&
      lastKEvent!=MyWidget::KeyEvent::K_Down ){
    msg.message( currentPlayer, Behavior::EditRotate, e.delta, 0 );
    } else {
    if( e.delta<0 )
      world->camera.setDistance( world->camera.distance() * 1.1 ); else
      world->camera.setDistance( world->camera.distance() / 1.1 );
    }
}

void Game::scutEvent(MyWidget::KeyEvent &e) {
  gui.scutEvent(e);
  }

void Game::keyDownEvent( MyWidget::KeyEvent &e ) {
  if( gui.keyDownEvent(e) )
    return;

  lastKEvent = e.key;
  }

void Game::keyUpEvent( MyWidget::KeyEvent & e ) {
  if( gui.keyUpEvent(e) )
    return;

  lastKEvent = MyWidget::KeyEvent::K_NoKey;
  }

void Game::toogleFullScr() {
  isFullScreen = !isFullScreen;
  toogleFullScreen( isFullScreen );
  }

void Game::setPlaylersCount(int c) {
  players.clear();

  ++c;
  for( int i=0; i<c; ++i )
    addPlayer();
  }

void Game::addEditorObject(const std::string &p, int pl, int x, int y,
                           size_t unitPl ) {
  GameObject *obj = &world->addObject( p, unitPl );

  obj->setPosition( x, y, 200 );
  obj->updatePos();

  if( player(pl).editObj )
    world->deleteObject(player(pl).editObj);

  player(pl).editObj = obj;
  }

void Game::moveEditorObject( int pl, int x, int y) {
  if( player(pl).editObj ){
    float wx = x/Terrain::quadSizef,
          wy = y/Terrain::quadSizef;

    GameObject & obj = *player(pl).editObj;
    obj.setPosition( x, y, obj.world().terrain().heightAt(wx,wy) );
    }
  }

void Game::rotateEditorObject(int pl, int x) {
  if( player(pl).editObj )
    player(pl).editObj->rotate( x/10 );
  }

void Game::nextEditorObject(int pl) {
  if( !player(pl).editObj )
    return;

  GameObject * obj = player(pl).editObj;
  player(pl).editObj = 0;
  addEditorObject( obj->getClass().name, pl, obj->x(), obj->y(),
                   obj->playerNum() );
  }

void Game::delEditorObject(int pl) {
  world->deleteObject( player(pl).editObj );
  player(pl).editObj = 0;
  }

bool Game::message( int pl,
                    BehaviorMSGQueue::Message m,
                    int x,
                    int y,
                    BehaviorMSGQueue::Modifers md ) {
  msg.message(pl, m, x, y, md);
  return 1;
  }

bool Game::instalHook(InputHookBase *b) {
  return gui.instalHook(b);
  }

void Game::removeHook(InputHookBase *b) {
  gui.removeHook(b);
  }

const ProtoObject &Game::prototype(const std::string &s) const {
  return proto.get(s);
 }

const Resource &Game::resources() const {
  return resource;
  }

void Game::addPlayer() {
  players.push_back( std::unique_ptr<Player>( new Player( players.size() ) ) );

  if( players.size() == 2 ){
    players[1]->setHostCtrl(1);
    players[1]->onUnitSelected.bind( gui, &MainGui::updateSelectUnits );
    players[1]->onUnitDied    .bind( gui, &MainGui::onUnitDied );
    }
  }

Player &Game::player(int i) {
  while( int(players.size())<=i )
    addPlayer();

  return *players[i];
  }

size_t Game::plCount() const {
  return players.size();
  }

Player &Game::player() {
  return player( currentPlayer );
  }

void Game::createEditorObject(const ProtoObject &p, int pl) {
  msg.message( currentPlayer,
               Behavior::EditAdd,
               World::coordCastD(world->camera.x()),
               World::coordCastD(world->camera.y()),
               p.name,
               pl );
  }

Game::F3 Game::unProject( int x, int y, float destZ ) {
  MyGL::Matrix4x4 mat = world->camera.projective();
  mat.mul( world->camera.view() );
  mat.inverse();

  double px =  2.0*(x-w/2.0)/double(w),
         py = -2.0*(y-h/2.0)/double(h);

  double vec1[4], vec2[4];
  mat.project( px, py, 0, 1,
               vec1[0], vec1[1], vec1[2], vec1[3] );
  mat.project( px, py, 1, 1,
               vec2[0], vec2[1], vec2[2], vec2[3] );

  for( int i=0; i<4; ++i ){
    vec1[i] /= vec1[3];
    vec2[i] /= vec2[3];
    }

  for( int i=0; i<4; ++i ){
    vec2[i] -= vec1[i];
    }

  double k = (vec1[2]-destZ)/vec2[2];
  for( int i=0; i<4; ++i ){
    vec1[i] -= k*vec2[i];
    }


  F3 r;
  std::copy( vec1, vec1+3, r.data );
  return r;
  }

Game::F3 Game::unProject(int x, int y) {
  float min = -2 + world->camera.z(),
        max =  2 + world->camera.z();
  F3 ret = unProject( x, y, 0 );
  float err = fabs( world->zAt(ret.data[0], ret.data[1]) - ret.data[2] );

  for( int i=0; i<20; ++i ){
    F3 v = unProject( x, y, min+(max-min)*i/20.0 );
    float err2 = fabs( world->zAt(v.data[0], v.data[1]) - v.data[2] );

    if( err2<err ){
      err = err2;
      ret = v;
      }
    }

  return ret;
  }

Game::F3 Game::project(float x, float y, float z) {
  MyGL::Matrix4x4 mat = world->camera.projective();
  mat.mul( world->camera.view() );
  //mat.transpose();

  F3 out;
  double data[4];
  mat.project( x,y,z, 1, data[0], data[1], data[2], data[3]);

  for( int i=0; i<4; ++i )
    data[i] /= data[3];

  std::copy( data, data+3, out.data );

  return out;
  }

void Game::moveCamera() {
  if( selectionRectTracking )
    return;

  const double cameraStep = 0.1;
  const int sensetive = 20;

  if( curMPos.x < sensetive || lastKEvent==MyWidget::KeyEvent::K_Left ){
    world->moveCamera( -cameraStep, 0 );
    }
  if( curMPos.x > w - sensetive || lastKEvent==MyWidget::KeyEvent::K_Right ){
    world->moveCamera( cameraStep, 0 );
    }

  if( curMPos.y < sensetive || lastKEvent==MyWidget::KeyEvent::K_Up ){
    world->moveCamera( 0, -cameraStep );
    }
  if( curMPos.y > h-sensetive || lastKEvent==MyWidget::KeyEvent::K_Down ){
    world->moveCamera( 0, cameraStep );
    }
  }

void Game::setCameraPos(GameObject &obj) {
  float x1 = World::coordCast(obj.x()),
        y1 = World::coordCast(obj.y()),
        z1 = World::coordCast(obj.z());

  float k = 0.3;
  float x = world->camera.x(),
        y = world->camera.y(),
        z = world->camera.z();

  world->camera.setPosition( x+(x1-x)*k, y+(y1-y)*k, z+(z1-z)*k );
  }

void Game::setupMaterials( MyGL::AbstractGraphicObject &obj,
                           const ProtoObject::View &src,
                           const MyGL::Color & teamColor ) {
  Resource &r = resource;
  GraphicsSystem::ObjectsClosure &c = graphics.closure;

  MainMaterial material( c.shadow.matrix );

  MyGL::ShadowMapPassMaterial smMaterial;

  material.diffuseTexture   = r.texture( src.name+"/diff" );
  material.normalMap        = r.texture( src.name+"/norm" );
  smMaterial.diffuseTexture = material.diffuseTexture;

  material.useAlphaTest = smMaterial.useAlphaTest = true;
  material.specular     = 1.0;

  if( contains( src.materials, "phong" )  ){
    material.specular = src.specularFactor;
    obj.setupMaterial( material );
    }

  if( contains( src.materials, "terrain.minor" )  ){
    TerrainMinorMaterial mat( c.shadow.matrix );
    mat.diffuseTexture = material.diffuseTexture;
    mat.normalMap      = material.normalMap;
    mat.useAlphaTest   = false;

    mat.specular = src.specularFactor;
    mat.alphaTrestRef = 0;
    obj.setupMaterial( mat );

    TerrainZPass tz;
    tz.texture = mat.diffuseTexture;
    obj.setupMaterial( tz );
    }

  if( contains( src.materials, "unit" )  ){
    MainMaterial material( c.shadow.matrix,
                           teamColor );
    material.diffuseTexture   = r.texture( src.name+"/diff" );
    material.normalMap        = r.texture( src.name+"/norm" );
    smMaterial.diffuseTexture = material.diffuseTexture;

    material.useAlphaTest = smMaterial.useAlphaTest = true;
    material.specular     = src.specularFactor;

    obj.setupMaterial( material );
    }

  if( contains( src.materials, "blush" )  ){
    BlushMaterial material( c.shadow.matrix );
    material.diffuseTexture   = r.texture( src.name+"/diff" );
    material.normalMap        = r.texture( src.name+"/norm" );
    smMaterial.diffuseTexture = material.diffuseTexture;

    material.useAlphaTest = smMaterial.useAlphaTest = true;
    material.specular     = src.specularFactor;

    obj.setupMaterial( material );

    if( contains( src.materials, "shadow_cast" ) ){
      BlushShMaterial sm;
      (MyGL::ShadowMapPassMaterial&)sm = smMaterial;

      if( r.findTexture(src.name+"/sm") )
        sm.diffuseTexture = r.texture( src.name+"/sm" );

      obj.setupMaterial( sm );
      }
    }

  if( contains( src.materials, "shadow_cast" ) &&
      !contains( src.materials, "blush" ) ){
    if( r.findTexture(src.name+"/sm") )
      smMaterial.diffuseTexture   = r.texture( src.name+"/sm" );

    if( contains( src.materials, "transparent_no_zw" ) ||
        contains( src.materials, "transparent" ))
      smMaterial.alphaTrestRef = 1.0f/255.0f;

    obj.setupMaterial( smMaterial );
    }

  if( contains( src.materials, "displace" ) ){
    DisplaceMaterial material( c.shadow.matrix );
    obj.setupMaterial( material );
    }

  if( contains( src.materials, "water" ) ){
    WaterMaterial material( c.shadow.matrix );
    material.texture = r.texture( "water/diff" );
    material.normals = r.texture( "water/norm" );

    obj.setupMaterial( material );
    }

  if( contains( src.materials, "glow" ) ){
    GlowMaterial material;
    material.texture = r.texture( src.name+"/glow" );

    obj.setupMaterial( material );
    }

  if( contains( src.materials, "add" ) ){
    AddMaterial material;
    material.texture = r.texture( src.name+"/diff" );

    obj.setupMaterial( material );
    }

  if( contains( src.materials, "transparent_no_zw" ) ){
    TransparentMaterialNoZW   material(c.shadow.matrix);
    material.texture   = r.texture( src.name+"/diff" );
    material.normalMap = r.texture( src.name+"/norm" );

    obj.setupMaterial( material );
    }

  if( contains( src.materials, "transparent" ) ){
    TransparentMaterialZPass zpass;
    TransparentMaterial      material(c.shadow.matrix);
    zpass.texture = r.texture( src.name+"/diff" );

    material.texture = zpass.texture;
    obj.setupMaterial( zpass    );
    obj.setupMaterial( material );
    }

  if( contains( src.materials, "omni" ) ){
    OmniMaterial material;
    obj.setupMaterial( material );
    }
  }

MyGL::Matrix4x4 &Game::shadowMat() {
  return graphics.closure.shadow.matrix;
  }


void Game::save( const std::wstring& f ) {
  std::string str;
  str.assign(f.begin(), f.end());

  GameSerializer s( str, Serialize::Write );

  if( s.isOpen() )
    serialize(s);
  }

void Game::load( const std::wstring& f ) {
  std::string str;
  str.assign(f.begin(), f.end());

  GameSerializer s( str, Serialize::Read );

  if( s.isOpen() )
    serialize(s);
  }

void Game::serialize( GameSerializer &s ) {
  std::string magic = "SAV";
  s + magic;

  if( magic!="SAV")
    return;

  if( s.isReader() ){
    worlds.clear();
    }

  int plCount = players.size()-1;
  s + plCount;
  s + currentPlayer;

  if( s.isReader() ){
    setPlaylersCount( plCount );
    }

  for( size_t i=0; i<players.size(); ++i )
    players[i]->serialize(s);

  int wCount = worlds.size(), curWorld = 0;
  s + wCount;

  for( size_t i=0; i<worlds.size(); ++i )
    if( worlds[i].get()==world )
      curWorld = i;
  s + curWorld;

  if( s.isReader() ){
    for( int i=0; i<wCount; ++i ){
      worlds.push_back( std::unique_ptr<World>( new World(graphics, resource,
                                                          proto,
                                                          *this,
                                                          128, 128) ) );
      world = worlds.back().get();
      world->camera.setPerespective( true, w, h );
      world->camera.setPosition( 2, 3, 0 );
      world->camera.setDistance( 4 );
      world->setupMaterial.bind(*this, &Game::setupMaterials );
      }
    }

  world = worlds[curWorld].get();

  gui.toogleEditLandMode = MyWidget::signal<const Terrain::EditMode&>();
  gui.toogleEditLandMode.bind( *world, &World::toogleEditLandMode );

  gui.paintObjectsHud = MyWidget::signal< MyWidget::Painter&, int, int>();
  gui.paintObjectsHud.bind( *world, &World::paintHUD );

  for( size_t i=0; i<worlds.size(); ++i )
    worlds[i]->serialize(s);
  }
