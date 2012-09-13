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

Game::Game( void *ihwnd, int iw, int ih, bool isFS )
  : graphics( ihwnd, iw, ih, isFS, isFS ? 2048:1024 ),
    resource( graphics.texHolder,
              graphics.vboHolder,
              graphics.iboHolder,
              graphics.vsHolder,
              graphics.fsHolder ),
    gui( graphics.device, iw, ih, resource, proto ),
    serializator("./serialize_tmp.obj", Serialize::Write ){
  w = iw;
  h = ih;
  isFullScreen = isFS;

  hwnd = ihwnd;
  currentPlayer = 1;

  spinX =  0;
  spinY = -150;
  mouseTracking         = 0;
  selectionRectTracking = 0;

  initFactorys();

  resource.load("./data/data.xml");
  proto   .load("./data/game.xml");

  setPlaylersCount(1);

  gui.createControls( msg );
  gui.enableHooks( !serializator.isReader() );
  gui.toogleFullScreen.bind( *this, &Game::toogleFullScr );
  gui.addObject.bind( *this, &Game::createEditorObject );

  graphics.load( resource, gui, w, h );

  worlds.push_back( std::unique_ptr<World>( new World(graphics, resource,
                                                      proto,
                                                      *this,
                                                      128, 128) ) );

  world = worlds[0].get();

  world->camera.setPerespective( true, w, h );
  world->camera.setPosition( 2, 3, 0 );
  world->camera.setDistance( 4 );

  gui.paintObjectsHud.bind( *world, &World::paintHUD );

  fps.n    = 0;
  fps.time = 0;
  }

void Game::tick() {
  DWORD time = GetTickCount();

  F3 v = unProject( curMPos.x, curMPos.y, 0 );
  int vx = World::coordCastD(v.data[0])/Terrain::quadSize,
      vy = World::coordCastD(v.data[1])/Terrain::quadSize;
  world->setMousePos( vx*Terrain::quadSize, vy*Terrain::quadSize );

  if( player().editObj ){
    msg.message( currentPlayer,
                 Behavior::EditMove,
                 vx*Terrain::quadSize,
                 vy*Terrain::quadSize );
    }

  msg.serialize( serializator );
  msg.tick( *this, *world );

  world->tick();

  fps.time += int(GetTickCount() - time);
  }

void Game::render() {
  world->camera.setSpinX(spinX);
  world->camera.setSpinY(spinY);

  DWORD time = GetTickCount();

  graphics.render( world->getScene(), world->camera );
  ++fps.n;
  fps.time += int(GetTickCount() - time);

  if( fps.n>100 ){
    double f = 1000.0*double(fps.n)/std::max(1, fps.time);
    SetWindowTextA( HWND(hwnd),
                    Lexical::upcast( f ).data() );

    fps.n    = 0;
    fps.time = 0;
    }
  //std::cout << GetTickCount() - time << std::endl;


  moveCamera();
  }

void Game::resizeEvent( int iw, int ih ){
  w = iw;
  h = ih;

  // std::cout << w << " " << h << std::endl;

  world->camera.setPerespective(true, w, h );

  graphics.resizeEvent( w, h, isFullScreen );
  gui.resizeEvent(w,h);
  }

void Game::mouseDownEvent( MyWidget::MouseEvent &e) {
  if( gui.mouseDownEvent(e) )
    return;

  mouseTracking         = (e.button==MyWidget::MouseEvent::ButtonRight);
  selectionRectTracking = (e.button==MyWidget::MouseEvent::ButtonLeft);

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

  mouseTracking         = false;
  selectionRectTracking = false;
  gui.selectionRect() = MyWidget::Rect(-1, -1, 0, 0);
  gui.update();

  F3 v = unProject( e.x, e.y, 0 );

  if( e.button==MyWidget::MouseEvent::ButtonLeft ){    
    world->updateSelectionFlag( msg, currentPlayer );
    }

  if( e.button==MyWidget::MouseEvent::ButtonRight ){
    msg.message( currentPlayer,
                 AbstractBehavior::Move,
                 World::coordCastD(v.data[0]),
                 World::coordCastD(v.data[1]) );
    }

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

  if( (player().editObj && !serializator.isReader() ) &&
      lastKEvent!=MyWidget::KeyEvent::K_Down ){
    msg.message( currentPlayer, Behavior::EditRotate, e.delta, 0 );
    } else {
    if( e.delta<0 )
      world->camera.setDistance( world->camera.distance() * 1.1 ); else
      world->camera.setDistance( world->camera.distance() / 1.1 );
    }
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

void Game::addEditorObject(const std::string &p, int pl, int x, int y ) {
  GameObject *obj = &world->addObject(p, pl);

  obj->setPosition( x, y, 200 );
  obj->updatePos();

  if( player(pl).editObj )
    world->removeObject(player(pl).editObj);

  player(pl).editObj = obj;
  }

void Game::moveEditorObject( int pl, int x, int y) {
  if( player(pl).editObj )
    player(pl).editObj->setPosition( x, y, 0 );
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
  addEditorObject( obj->getClass().name, pl, obj->x(), obj->y() );
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
    }
  }

Player &Game::player(int i) {
  while( int(players.size())<=i )
    addPlayer();

  return *players[i];
  }

Player &Game::player() {
  return player( currentPlayer );
  }

void Game::createEditorObject(const ProtoObject &p) {
  msg.message( currentPlayer,
               Behavior::EditAdd,
               World::coordCastD(world->camera.x()),
               World::coordCastD(world->camera.y()),
               p.name );
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

  double k = vec1[2]/vec2[2];
  for( int i=0; i<4; ++i ){
    vec1[i] -= k*vec2[i];
    }


  F3 r;
  std::copy( vec1, vec1+3, r.data );
  return r;
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

void Game::setupMaterials( MyGL::AbstractGraphicObject &obj,
                           const ProtoObject::View &src,
                           const MyGL::Color & teamColor ) {
  Resource &r = resource;
  GraphicsSystem::ObjectsClosure &c = graphics.closure;

  MyGL::GBufferFillPass::Material
      material( c.shadowMap->shadowMap().surface(),
                c.shadow.matrix );

  MyGL::ShadowMapPassBase::Material smMaterial;

  material.diffuseTexture   = r.texture( src.name+"/diff" );
  smMaterial.diffuseTexture = material.diffuseTexture;

  material.useAlphaTest = smMaterial.useAlphaTest = true;
  material.specular     = 1.0;

  if( contains( src.materials, "phong" )  ){
    material.specular = src.specularFactor;
    obj.setupMaterial( material );
    }

  if( contains( src.materials, "unit" )  ){
    MainMaterial material( c.shadowMap->shadowMap().surface(),
                           c.shadow.matrix,
                           teamColor );
    material.diffuseTexture   = r.texture( src.name+"/diff" );
    smMaterial.diffuseTexture = material.diffuseTexture;

    material.useAlphaTest = smMaterial.useAlphaTest = true;
    material.specular     = src.specularFactor;

    obj.setupMaterial( material );
    }

  if( contains( src.materials, "shadow_cast" ) ){
    if( r.findTexture(src.name+"/sm") )
      smMaterial.diffuseTexture   = r.texture( src.name+"/sm" );

    obj.setupMaterial( smMaterial );
    }

  if( contains( src.materials, "displace" ) ){
    DisplaceMaterial
        material( c.shadowMap->shadowMap().surface(),
                  c.shadow.matrix );

    obj.setupMaterial( material );
    }

  if( contains( src.materials, "water" ) ){
    WaterMaterial
        material( c.shadowMap->shadowMap().surface(),
                  c.shadow.matrix );
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

  if( contains( src.materials, "transparent" ) ){
    TransparentMaterialZPass zpass;
    TransparentMaterial      material;
    zpass.texture = r.texture( src.name+"/diff" );

    material = zpass;
    obj.setupMaterial( zpass    );
    obj.setupMaterial( material );
    }
  }
