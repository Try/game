#include "game.h"

#include <Tempest/TessObject>
#include <Tempest/Event>
#include <iostream>

#include "threads/time.h"

#include "util/lexicalcast.h"
#include "util/factory.h"

#include "util/serialize.h"
#include "algo/algo.h"
#include <Tempest/Pixmap>

#include "util/gameserializer.h"

#include "behavior/buildingbehavior.h"

#include "network/localserver.h"
#include "network/client.h"

#include "game/missions/scenariomission1.h"
#include "game/missions/deatmachscenarion.h"

#include <cmath>

Game::Game( ShowMode sm )
     :Window(sm),
      graphics( handle(), isFullScreenMode(), isFullScreenMode() ? 2048:1024 ),
      soundDev( handle() ),
      resource( graphics.texHolder,
                graphics.localTex,
                graphics.vboHolder,
                graphics.lvboHolder,
                graphics.iboHolder,
                graphics.vsHolder,
                graphics.fsHolder ),
      gui( graphics.device, w(), h(), resource, proto ),
      msg(*this),
      serializator(L"./serialize_tmp.obj", Serialize::Write ){
  //w = iw;
  //h = ih;
  //isFullScreen = isFS;
  paused = false;

  acceptMouseObj = true;

  curMPos = Tempest::Point(w()/2, h()/2);

  currentPlayer = 1;

  mouseTracking         = 0;
  selectionRectTracking = 0;

  initFactorys();

  /*
  Tempest::Model<>::Raw m = Tempest::Model<>::loadRawData("./data/models/grass/0.mx");

  float tc[6][2] = {
    {0, 0},
    {0, 1},
    {1, 1},

    {0, 0},
    {1, 1},
    {1, 0}
    };

  m.vertex.resize(150*6);
  for( size_t i=0; i<m.vertex.size(); i+=6 ){
    float cx = ((rand()/float(RAND_MAX)) - 0.5)*16;
    float cy = ((rand()/float(RAND_MAX)) - 0.5)*16;

    float a = (rand()/float(RAND_MAX))*2*M_PI;

    int x = rand()%2, y = rand()%3;

    for( int r=0; r<6; ++r ){
      m.vertex[i+r].u = (x+tc[r][0])*0.5;
      m.vertex[i+r].v = 1 - (y+tc[r][1])/3.0;

      m.vertex[i+r].x = cx + cos(a)*(tc[r][0]*2-1)*0.8;
      m.vertex[i+r].y = cy + sin(a)*(tc[r][0]*2-1)*0.8;
      m.vertex[i+r].z = tc[r][1]*2.0;
      }
    }

  size_t sz = m.vertex.size();
  for( size_t i=0; i<sz; i+=3 ){
    m.vertex.push_back( m.vertex[i] );
    m.vertex.push_back( m.vertex[i+2] );
    m.vertex.push_back( m.vertex[i+1] );
    }

  Tempest::Model<>::saveRawData("./data/models/grass/0.mx", m);
  */

  resource.load("./data/data.xml");
  proto   .load("./data/game.xml");

  mscenario.reset( new DeatmachScenarion() );

  //Tempest::Model<>::saveRawData( "./sphere.mx", Tempest::TessObject::sphere(3, 1) );

  gui.createControls( msg, *this );
  gui.enableHooks( !serializator.isReader() );
  gui.toogleFullScreen.bind( *this, &Game::toogleFullScr );
  gui.addObject.bind( *this, &Game::createEditorObject );
  gui.setCameraPos.bind(*this, &Game::setCameraPos );
  gui.minimapEvent.bind(*this, &Game::minimapEvent );

  gui.save.bind( *this, &Game::save );
  gui.load.bind( *this, &Game::load );

  graphics.load( resource, gui, w(), h() );
  graphics.onRender.bind( *this, &Game::onRender );

  gui.renderScene.bind( graphics, &GraphicsSystem::renderSubScene );

  worlds.push_back( std::shared_ptr<World>( new World(*this,
                                                      256, 256) ) );

  world = worlds[0].get();
  world->camera.setPerespective( true, w(), h() );
  world->setupMaterial.bind(*this, &Game::setupMaterials );

  gui.toogleEditLandMode.bind( *world, &World::toogleEditLandMode );
  gui.paintObjectsHud.bind( *world, &World::paintHUD );

  fps.n    = 0;
  fps.time = 0;

  sendDelay = 0;

  //resource.sound("hammer0").play();

  setPlaylersCount(1);

  load(L"./campagin/0.sav");
  mscenario->onStartGame();
  }

void Game::tick() {
  //return;

  moveCamera();

  size_t time = Time::tickCount();

  F3 v = unProject( curMPos.x, curMPos.y );
  int vx = World::coordCastD(v.data[0])/Terrain::quadSize,
      vy = World::coordCastD(v.data[1])/Terrain::quadSize;

  world->setMousePos( vx*Terrain::quadSize,
                      vy*Terrain::quadSize,
                      World::coordCastD(v.data[2]) );
  if( acceptMouseObj ){
    world->setMouseObject( world->unitUnderMouse( curMPos.x,
                                                  curMPos.y,
                                                  w(), h() ) );
    } else {
    world->setMouseObject( -1 );
    }

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

  bool isLag = false;
  ++sendDelay;

  if( netUser ){
    if( netUser->isConnected() &&
        sendDelay >=10 &&
        !paused  ){
      isLag = !msg.syncByNet( *netUser );

      if( !isLag ){
        msg.serialize( serializator );
        msg.tick( *this, *world );
        sendDelay = 0;
        }
      }
    } else {
    if( !paused ){
      msg.serialize( serializator );
      msg.tick( *this, *world );
      }
    }

  if( !isLag && !paused ){
    for( size_t i=0; i<players.size(); ++i )
      players[i]->computeFog(world);

    world->tick();
    mscenario->tick();
    }

  fps.time += int(Time::tickCount() - time);
  }

void Game::onRender( double dt ){  
  World::CameraViewBounds b;
  F3 vb[4];
  vb[0] = unProject(   0, 0   );
  vb[1] = unProject( w(), 0   );
  vb[2] = unProject(   0, h() );
  vb[3] = unProject( w(), h() );

  for( int i=0; i<4; ++i ){
    b.x[i] = World::coordCastD(vb[i].data[0]);
    b.y[i] = World::coordCastD(vb[i].data[1]);
    }

  world->setCameraBounds(b);
  gui.renderMinimap(*world);

  if( gui.isCutsceneMode() ){
    Tempest::Pixmap p(1,1, false);
    Tempest::Pixmap::Pixel pix;
    pix.r = 255;
    pix.g = 255;
    pix.b = 255;
    pix.a = 255;

    p.set(0,0, pix);
    graphics.setFog( p );
    } else {
    graphics.setFog( player().fog() );
    }

  world->onRender(dt);
  }

void Game::render() {
  tick();//FIXME

  gui.updateValues();

  //world->camera.setSpinX(spinX);
  //world->camera.setSpinY(spinY);

  size_t time = Time::tickCount();
  size_t dt   = time;

  if( graphics.render( world->getScene(),
                       world->getParticles(),
                       world->camera,
                       dt )){

    }

  ++fps.n;
  fps.time += int(Time::tickCount() - time);

  if( fps.n>100 || ( fps.n>0 && fps.time>1000 ) ){
    double f = 1000.0*double(fps.n)/std::max(1, fps.time);
#ifdef __WIN32
    SetWindowTextA( HWND( handle() ),
                    Lexical::upcast( f ).data() );
#endif
    fps.n    = 0;
    fps.time = 0;
    }

  }

void Game::resizeEvent( Tempest::SizeEvent &e ){
  //w = e.w;
  //h = e.h;

  // std::cout << w << " " << h << std::endl;

  world->camera.setPerespective(true, w(), h() );

  graphics.resizeEvent( e.w, e.h, isFullScreenMode() );
  gui.resizeEvent( e.w, e.h );
  }

void Game::mouseDownEvent( Tempest::MouseEvent &e) {
  if( gui.mouseDownEvent(e) )
    return;

  gui.setFocus();
  mouseTracking         = (e.button==Tempest::MouseEvent::ButtonRight);
  if(e.button==Tempest::MouseEvent::ButtonLeft)
    selectionRectTracking = 1;

  gui.selectionRect() = Tempest::Rect(e.x, e.y, 0, 0);

  lastMPos = Tempest::Point( e.x, e.y );

  if( e.button==Tempest::MouseEvent::ButtonLeft && player().editObj ){
    msg.message( currentPlayer, Behavior::EditNext );
    }

  if( e.button==Tempest::MouseEvent::ButtonRight && player().editObj ){
    msg.message( currentPlayer, Behavior::EditDel );
    }
  }

void Game::mouseUpEvent( Tempest::MouseEvent &e) {
  if( !selectionRectTracking && gui.mouseUpEvent(e) )
    return;

  gui.setFocus();

  mouseTracking         = false;
  gui.selectionRect() = Tempest::Rect(-1, -1, 0, 0);
  gui.update();

  F3 v = unProject( e.x, e.y );

  if( player().editObj==0 ){
    if( e.button==Tempest::MouseEvent::ButtonLeft ){
      if( selectionRectTracking==2 )
        world->updateSelectionFlag( msg, currentPlayer ); else
        world->updateSelectionClick( msg, currentPlayer, e.x, e.y,
                                     w(), h() );
      }

    if( e.button==Tempest::MouseEvent::ButtonRight ){
      size_t obj = world->unitUnderMouse( e.x, e.y,
                                          w(), h() );

      if( obj==size_t(-1) ){
        msg.message( currentPlayer,
                     AbstractBehavior::Move,
                     World::coordCastD(v.data[0]),
                     World::coordCastD(v.data[1]) );
        } else {
        msg.message_st( currentPlayer,
                        AbstractBehavior::ToUnit,
                        obj );
        }
      }
    }

  selectionRectTracking = false;
  world->clickEvent( World::coordCastD(v.data[0]),
                     World::coordCastD(v.data[1]),
                     e );
  }

void Game::mouseMoveEvent( Tempest::MouseEvent &e ) {
  curMPos = Tempest::Point(e.x, e.y);

  if( !selectionRectTracking && gui.mouseMoveEvent(e) ){
    acceptMouseObj = false;
    return;
    }

  acceptMouseObj = true;

  if( mouseTracking ){
    //world->camera.setSpinX( world->camera.spinX() - (e.x - lastMPos.x) );
    //world->camera.setSpinY( world->camera.spinY() - (e.y - lastMPos.y) );

    lastMPos = Tempest::Point(e.x, e.y);
    }

  if( selectionRectTracking ){
    selectionRectTracking = 2;

    Tempest::Rect & r = gui.selectionRect();
    r.w = e.x - gui.selectionRect().x;
    r.h = e.y - gui.selectionRect().y;

    //gui.update();

    double ww = w()/2.0, hh = h()/2.0;
    world->updateMouseOverFlag(    r.x/ww - 1.0,
                                 -(r.y/hh - 1.0),
                                  (r.x+r.w)/ww - 1.0,
                                -((r.y+r.h)/hh - 1.0) );
    }

  }

void Game::mouseWheelEvent( Tempest::MouseEvent &e ) {
  if( gui.mouseWheelEvent(e) ){
    return;
    }
  gui.setFocus();

  if( (player().editObj && !serializator.isReader() ) &&
      lastKEvent!=Tempest::KeyEvent::K_Down ){
    int dR = 10;
    if( e.delta<0 )
      dR = -10;

    msg.message( currentPlayer, Behavior::EditRotate, dR, 0 );
    } else {
    if( e.delta<0 )
      world->camera.setDistance( world->camera.distance() * 1.1 ); else
      world->camera.setDistance( world->camera.distance() / 1.1 );
    }
}

void Game::shortcutEvent(Tempest::KeyEvent &e) {
  gui.scutEvent(e);
  }

void Game::keyDownEvent( Tempest::KeyEvent &e ) {
  if( gui.keyDownEvent(e) )
    return;

  lastKEvent = e.key;
  }

void Game::keyUpEvent( Tempest::KeyEvent & e ) {
  if( gui.keyUpEvent(e) )
    return;

  lastKEvent = Tempest::KeyEvent::K_NoKey;
  }

void Game::toogleFullScr() {
  //isFullScreen = !isFullScreen;
  toogleFullScreen( isFullScreenMode() );
  }

void Game::setPlaylersCount(int c) {
  players.clear();

  ++c;
  for( int i=0; i<c; ++i )
    addPlayer();
  }

void Game::addEditorObject(const std::string &p,
                           int pl, int x, int y,
                           int rAngle,
                           size_t unitPl ) {
  GameObject *obj = &world->addObject( p, unitPl );

  obj->setPosition( x, y, 200 );

  if( obj->getClass().view.size() && !obj->getClass().view[0].randRotate )
    obj->rotate( rAngle );

  obj->updatePos();

  if( player(pl).editObj )
    world->deleteObject(player(pl).editObj);

  world->clrUpdateIntents();
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
    player(pl).editObj->rotate( x );
  }

void Game::nextEditorObject(int pl) {
  if( !player(pl).editObj )
    return;

  GameObject * obj = player(pl).editObj;
  player(pl).editObj = 0;
  addEditorObject( obj->getClass().name, pl,
                   obj->x(), obj->y(),
                   180+obj->rAngle()*180/M_PI,
                   obj->playerNum() );
  }

void Game::delEditorObject(int pl) {
  world->deleteObject( player(pl).editObj );
  world->clrUpdateIntents();
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

bool Game::message( int pl,
                    AbstractBehavior::Message m,
                    int x,
                    int y,
                    const std::string &spell,
                    AbstractBehavior::Modifers md ) {
  msg.message(pl, m, x, y, spell, 0, md);
  return 1;
  }

bool Game::message( int pl,
                    AbstractBehavior::Message m,
                    size_t id,
                    const std::string &spell,
                    AbstractBehavior::Modifers md ) {
  msg.message(pl, m, 0, 0, spell, id, md);
  return 1;
  }

bool Game::message( int pl,
                    BehaviorMSGQueue::Message m,
                    size_t id,
                    BehaviorMSGQueue::Modifers /*md*/ ) {
  msg.message_st(pl, m, id, 1);
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

Resource &Game::resources() {
  return resource;
  }

const PrototypesLoader &Game::prototypes() const {
  return proto;
  }

void Game::addPlayer() {
  players.push_back( std::shared_ptr<Player>( new Player( players.size() ) ) );

  if( players.size() == 2 ){
    players[1]->setHostCtrl(1);
    }

  players.back()->onUnitSelected.bind( *this, &Game::onUnitsSelected );
  players.back()->onUnitDied    .bind( *this, &Game::onUnitDied );
  //if( world )
    //players.back()->computeFog(world);
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

void Game::onUnitsSelected( std::vector<GameObject *> &u,
                            Player & pl ) {
  if( pl.hasHostControl() ){
    gui.updateSelectUnits(u);
    }
  }

void Game::onUnitDied( GameObject& u, Player & pl ) {
  if( pl.hasHostControl() ){
    gui.onUnitDied(u);
    }
  }

Game::F3 Game::unProject( int x, int y, float destZ ) {
  Tempest::Matrix4x4 mat = world->camera.projective();
  mat.mul( world->camera.view() );
  mat.inverse();

  double px =  2.0*(x-w()/2.0)/double(w()),
         py = -2.0*(y-h()/2.0)/double(h());

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
  Tempest::Matrix4x4 mat = world->camera.projective();
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

  if( curMPos.x < sensetive || lastKEvent==Tempest::KeyEvent::K_Left ){
    world->moveCamera( -cameraStep, 0 );
    }
  if( curMPos.x > w() - sensetive || lastKEvent==Tempest::KeyEvent::K_Right ){
    world->moveCamera( cameraStep, 0 );
    }

  if( curMPos.y < sensetive || lastKEvent==Tempest::KeyEvent::K_Up ){
    world->moveCamera( 0, -cameraStep );
    }
  if( curMPos.y > h() - sensetive || lastKEvent==Tempest::KeyEvent::K_Down ){
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

  float l0 = sqrt( pow(x-x1,2) + pow(y-y1,2) + pow(z-z1,2) );
  l0 = std::max(0.0f, l0-1);

  k = std::min(0.3+l0, 1.0);

  world->camera.setPosition( x+(x1-x)*k, y+(y1-y)*k, z+(z1-z)*k );
  world->moveCamera(0,0);
  }

void Game::minimapEvent( float fx, float fy,
                         Tempest::Event::MouseButton b,
                         MiniMapView::Mode m ) {
  world->setMousePos( fx*world->terrain().width() *Terrain::quadSizef,
                      fy*world->terrain().height()*Terrain::quadSizef,
                      0 );
  if( gui.minimapMouseEvent( fx*world->terrain().width() *Terrain::quadSizef,
                             fy*world->terrain().height()*Terrain::quadSizef,
                             b,
                             m ) ){
    return;
    }

  float x1 = World::coordCast(fx*world->terrain().width() *Terrain::quadSizef),
        y1 = World::coordCast(fy*world->terrain().height()*Terrain::quadSizef),
        z1 = world->camera.z();

  if( b==Tempest::MouseEvent::ButtonLeft ){
    float k = 1;//0.3;
    float x = world->camera.x(),
          y = world->camera.y(),
          z = world->camera.z();

    world->camera.setPosition( x+(x1-x)*k, y+(y1-y)*k, z+(z1-z)*k );
    world->moveCamera(0,0);
    }

  if( b==Tempest::MouseEvent::ButtonRight ){
    msg.message( currentPlayer,
                 AbstractBehavior::Move,
                 fx*world->terrain().width() *Terrain::quadSizef,
                 fy*world->terrain().height()*Terrain::quadSizef );
    }
  }

Scenario &Game::scenario() {
  return *mscenario;
  }

void Game::setupMaterials( AbstractGraphicObject &obj,
                           const ProtoObject::View &src,
                           const Tempest::Color & teamColor ) {
  Resource &r = resource;

  Material material;

  material.diffuse   = r.texture( src.name+"/diff" );
  material.normal    = r.texture( src.name+"/norm" );
  material.specular  = src.specularFactor;

  if( contains( src.materials, "phong" ) ||
      contains( src.materials, "unit" )  ||
      contains( src.materials, "blush" ) ){
    material.usage.mainPass = true;
    }

  if( contains( src.materials, "unit" ) ){
    material.useAlphaTest = 0;
    material.teamColor = &teamColor;
    }

  if( contains( src.materials, "shadow_cast" ) ){
    material.usage.shadowCast = true;
    }

  if( contains( src.materials, "glow" ) ){
    material.glow = r.texture( src.name+"/glow" );
    }

  if( contains( src.materials, "terrain.minor" ) ){
    material.usage.terrainMinor = true;
    }

  if( contains( src.materials, "displace" ) ){
    material.usage.displace = true;
    }

  if( contains( src.materials, "water" ) ){
    material.usage.water = true;
    }

  if( contains( src.materials, "blush" ) ||
      contains( src.materials, "grass" ) ){
    material.usage.blush = true;
    }

  if( contains( src.materials, "grass" ) ){
    material.usage.grass   = true;
    material.usage.blush   = true;
    material.alphaTrestRef = 0;
    }

  if( contains( src.materials, "add" ) ){
    material.usage.add = true;
    }

  if( contains( src.materials, "transparent" ) ){
    material.usage.transparent = true;
    }

  if( contains( src.materials, "transparent_no_zw" ) ){
    material.usage.transparent = true;
    material.zWrighting        = false;
    }

  if( contains( src.materials, "fog_of_war" ) ){
    material.usage.fogOfWar = true;
    }

  if( contains( src.materials, "omni" ) ){
    material.usage.omni = true;
    }

  obj.setMaterial( material );
  }

Tempest::Matrix4x4 &Game::shadowMat() {
  return graphics.closure.shadow.matrix;
  }


void Game::save( const std::wstring& f ) {
  GameSerializer s( f, Serialize::Write );

  if( s.isOpen() )
    serialize(s);
  }

void Game::load( const std::wstring& f ) {
  GameSerializer s( f, Serialize::Read );

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
    world = 0;
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
      worlds.push_back( std::shared_ptr<World>( new World(*this,
                                                          128, 128) ) );
      world = worlds.back().get();
      world->camera.setPerespective( true, w(), h() );
      world->camera.setPosition( 2, 3, 0 );
      world->camera.setDistance( 4 );
      world->setupMaterial.bind(*this, &Game::setupMaterials );
      }
    }

  world = worlds[curWorld].get();

  gui.toogleEditLandMode = Tempest::signal<const Terrain::EditMode&>();
  gui.toogleEditLandMode.bind( *world, &World::toogleEditLandMode );

  gui.paintObjectsHud = Tempest::signal< Tempest::Painter&, int, int>();
  gui.paintObjectsHud.bind( *world, &World::paintHUD );

  for( size_t i=0; i<worlds.size(); ++i )
    worlds[i]->serialize(s);

  for( size_t i=0; i<players.size(); ++i )
    players[i]->computeFog(world);

  if( s.version()>=7 ){
    bool isScenario = mscenario->isCampagin();
    s + isScenario;

    if( s.isReader() ){
      if( isScenario ){
        mscenario.reset( new ScenarioMission1(*this, gui) );
        } else {
        mscenario.reset( new DeatmachScenarion() );
        }
      }
    mscenario->serialize(s);
    } else {
    mscenario.reset( new DeatmachScenarion() );
    //mscenario.reset( new ScenarioMission1(*this, gui) );
    mscenario->serialize(s);
    }

  updateMissionTargets();
  }

void Game::log(const std::string &l) {
  std::cout << l << std::endl;
  }

void Game::setupAsServer() {
  LocalServer *s = new LocalServer();
  netUser.reset( s );
  netUser->onRecv.bind( msg,   &BehaviorMSGQueue::onRecvSrv );
  netUser->onError.bind( *this, &Game::log );
  s->onConnected.   bind( msg, &BehaviorMSGQueue::onNewClient );
  s->onDisConnected.bind( msg, &BehaviorMSGQueue::onDelClient );

  netUser->start();
  sendDelay = 0;
  }

void Game::setupAsClient(const std::wstring &s ) {
  Client * c = new Client();
  netUser.reset( c );
  netUser->onRecv.bind( msg,   &BehaviorMSGQueue::onRecvClient );
  netUser->onError.bind( *this, &Game::log );
  netUser->start();

  std::string str;
  str.assign( s.begin(), s.end() );

  c->connect( str );
  sendDelay = 0;
  }

void Game::onUnitRemove(size_t i) {
  msg.onUnitRemove(i);
  }

void Game::pause(bool p) {
  paused = p;
  }

void Game::unsetPause() {
  pause(0);
  }

bool Game::isPaused() const {
  return paused;
  }

void Game::setCurrectPlayer(int pl) {
  player(currentPlayer).setHostCtrl(0);
  currentPlayer = pl;
  player(currentPlayer).setHostCtrl(1);
  }
