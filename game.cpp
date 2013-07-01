#include "game.h"

#include <Tempest/TessObject>
#include <Tempest/Event>
#include <Tempest/Application>
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
#include "game/missions/deatmachscenario.h"
#include "game/missions/desertstrikescenario.h"

#include "util/scenariofactory.h"

#include <cmath>
#include "gui/gamemessages.h"

const int Game::ticksPerSecond = 35;

Game::Game( ShowMode sm )
     :Window(sm),
      graphics( handle(), isFullScreenMode() ),
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
  paused       = false;
  needToUpdate = false;

  currentPlayer = 1;

  //size_t s = sizeof(MVertex);

  initFactorys();
  loadData();

  //graphics.drawOpWindow.bind( this, &Game::update);

  isRunning        = true;
  physicStarted    = false;

  physicCompute = async( this, &Game::computePhysic, 0 );
  }

Game::~Game() {
  gui.setupMinimap(0);
  isRunning = false;
  physicCompute.join();
  worlds.clear();
  }

void Game::loadData() {
  resource.load("data/data.xml");
  proto   .load("data/game.xml");
  world = 0;

  setScenario( new DesertStrikeScenario(*this, gui, msg) );

  gui.toogleFullScreen.bind( *this, &Game::toogleFullScr );
  gui.addObject.bind( *this, &Game::createEditorObject );
  gui.onSetPlayer.bind( this, &Game::setEditorObjectPl );
  gui.toogleEditTab.bind( this, &Game::cancelEdit );

  gui.setCameraPos.bind( this, &Game::setCameraPos );
  gui.minimapEvent.bind( this, &Game::minimapEvent );

  gui.onSettingsChanged.bind( *this, &Game::settingsChanged );

  gui.save.bind( *this, &Game::save );
  gui.load.bind( *this, &Game::load );

  graphics.load( resource, gui, w(), h() );
  graphics.onRender.bind( *this, &Game::onRender );

  gui.renderScene.bind( graphics, &GraphicsSystem::renderSubScene );

  worlds.push_back( std::shared_ptr<World>( new World(*this,
                                                      128, 128) ) );

  world = worlds[0].get();

  world->camera.setPerespective( true, w(), h() );
  world->setupMaterial.bind( this, &Game::setupMaterials );

  gui.toogleEditLandMode.bind( *world, &World::toogleEditLandMode );
  gui.paintObjectsHud.bind( *world, &World::paintHUD );

  fps.n    = 0;
  fps.time = 0;

  sendDelay = 0;

  //resource.sound("hammer0").play();

  setPlaylersCount(1);

#ifdef __ANDROID__
  loadMission("campagin/td2.sav");
  setScenario( new DesertStrikeScenario(*this, gui, msg) );
#else
  loadMission("save/td2.sav");
  setScenario( new DesertStrikeScenario(*this, gui, msg) );
#endif
  //loadPngWorld( Tempest::Pixmap("./terrImg/h2.png") );

  setScenario( new DesertStrikeScenario(*this, gui, msg) );
  //setScenario( new DeatmachScenario(*this, gui, msg) );

  //for( size_t i=0; i<world->activeObjects().size(); ++i )
    //world->activeObjects()[i]->setHP(0);

  mscenario->onStartGame();
  updateTime = Time::tickCount();
  }

void Game::loadPngWorld( const Tempest::Pixmap& png ){
  world = 0;
  gui.setupMinimap(0);
  worlds.clear();
  worlds.push_back( std::shared_ptr<World>( new World(*this,
                                                      png.width(), png.height()) ) );

  world = worlds[0].get();

  world->camera.setPerespective( true, w(), h() );
  world->setupMaterial.bind( this, &Game::setupMaterials );

  gui.toogleEditLandMode = Tempest::signal<const Terrain::EditMode&>();
  gui.toogleEditLandMode.bind( *world, &World::toogleEditLandMode );

  gui.paintObjectsHud = Tempest::signal< Tempest::Painter&, int, int>();
  gui.paintObjectsHud.bind( *world, &World::paintHUD );

  world->terrain().loadFromPixmap( png );
  gui.setupMinimap(world);
  }

void Game::setScenario(Scenario *s) {
  mscenario.reset( s );

  gui.createControls( msg, *this );
  gui.enableHooks( !serializator.isReader() );

  gui.setupMinimap(world);

  for( size_t i=0; i<worlds.size(); ++i )
    for( size_t r=0; r<worlds[i]->objectsCount(); ++r ){
      Player& pl = player( worlds[i]->object(r).playerNum() );
      pl.addUnit( &worlds[i]->object(r) );
      }
  }

void Game::computePhysic(void *) {
  while( isRunning ){
    if( world ){
      if( !isPaused() )
        world->physics.tick(1);
      physicStarted = true;
      }

    Time::sleep(60);
    }
  }

void Game::tick() {
  scenario().uiTick();

  if( !isPaused() )
    GameMessages::tickAll();

  //size_t time = Time::tickCount();

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
    for( int i=0; i<plCount(); ++i )
      player().computeFog(world);

    world->tick();
    mscenario->tick();
    }

  //fps.time += int(Time::tickCount() - time);
  }

void Game::onRender( double dt ){  
  World::CameraViewBounds b;
  F3 vb[4];
  vb[0] = scenario().unProject(   0, 0   );
  vb[1] = scenario().unProject( w(), 0   );
  vb[2] = scenario().unProject(   0, h() );
  vb[3] = scenario().unProject( w(), h() );

  for( int i=0; i<4; ++i ){
    b.x[i] = World::coordCastD(vb[i].data[0]);
    b.y[i] = World::coordCastD(vb[i].data[1]);
    }

  world->setCameraBounds(b);
  gui.renderMinimap();

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

void Game::update(){
  if( !needToUpdate )
    return;
  needToUpdate = false;

  static const size_t updateDT = 1000/ticksPerSecond;
  size_t tnow = Time::tickCount();
  if( (tnow-updateTime)/updateDT>0 ){
    int c = (tnow-updateTime)/updateDT;
    int cx = std::min(c,3);

    for( ; cx; --cx ){
      updateTime += updateDT;
      tick();
      }

    if( c>3 ){
      updateTime = tnow;
      }
    {
      if( !physicStarted ){
        //Time::sleep(60);
        if( !isPaused() )
          world->physics.tick(c);
        //return;
        }
      }
    }

  //size_t dt   = time;
  }

void Game::render() {
  needToUpdate = true;

  size_t time = Time::tickCount();
  if( graphics.render( world->getScene(),
                       world->getParticles(),
                       world->camera,
                       Time::tickCount() )){

    }
  update();

  ++fps.n;
  fps.time += int(Time::tickCount() - time);

  if( fps.n>100 || ( fps.n>0 && fps.time>1000 ) ){
    double f = 1000.0*double(fps.n)/std::max(1, fps.time);

    gui.setFPS( f );

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

  scenario().mouseDownEvent(e);
  }

void Game::mouseUpEvent( Tempest::MouseEvent &e) {
  if( !scenario().isSelectionRectTracking() && gui.mouseUpEvent(e) )
    return;

  gui.setFocus();

  scenario().mouseUpEvent(e);
  }

void Game::mouseMoveEvent( Tempest::MouseEvent &e ) {
  scenario().mouseMoveEvent(e);
  }

void Game::mouseWheelEvent( Tempest::MouseEvent &e ) {
  if( gui.mouseWheelEvent(e) ){
    return;
    }
  gui.setFocus();
  scenario().mouseWheelEvent(e);
  }

void Game::shortcutEvent(Tempest::KeyEvent &e) {
  gui.scutEvent(e);
  }

void Game::keyDownEvent( Tempest::KeyEvent &e ) {
  if( gui.keyDownEvent(e) )
    return;

  scenario().keyDownEvent(e);
  }

void Game::keyUpEvent( Tempest::KeyEvent & e ) {
  if( gui.keyUpEvent(e) )
    return;

  scenario().keyUpEvent(e);

  if( e.key==Tempest::Event::K_Delete ){
    world->camera.setSpinX( world->camera.spinX()+10 );
    }
  if( e.key==Tempest::Event::K_Insert ){
    world->camera.setSpinX( world->camera.spinX()-10 );
    }
  }

void Game::toogleFullScr() {
  toogleFullScreen( isFullScreenMode() );
  }

void Game::setPlaylersCount(int c) {
  scenario().setPlaylersCount(c);
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

PrototypesLoader &Game::prototypes() {
  return proto;
  }

void Game::addPlayer() {
  scenario().addPlayer();
  }

void Game::cancelEdit(int) {
  if( player().editObj ){
    msg.message( currentPlayer, Behavior::EditDel );
    }
  }

Player &Game::player(int i) {
  return scenario().player(i);
  }

int Game::plCount() const {
  return scenario().plCount();
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

void Game::setEditorObjectPl(int pl) {
  if( player().editObj ){
    msg.message( currentPlayer,
                 Behavior::EditAdd,
                 player().editObj->x(),
                 player().editObj->y(),
                 player().editObj->getClass().name,
                 pl );
    }
  }

void Game::onUnitsSelected( std::vector<GameObject *> &u,
                            Player & pl ) {
  if( pl.hasHostControl() ){
    gui.updateSelectUnits(u);
    }
  }

void Game::onUnitDied( GameObject& u, Player & pl ) {
  if( pl.hasHostControl() && mscenario ){
    gui.onUnitDied(u);
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

const Scenario &Game::scenario() const {
  return *mscenario;
  }

World &Game::curWorld() {
  return *world;
  }

bool Game::isReplayMode() const {
  return serializator.isReader();
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
      contains( src.materials, "unit"  ) ){
    material.usage.mainPass = true;
    }

  if( contains( src.materials, "phong_atest" ) ){
    material.usage.mainPassAtst = true;
    }

  if( contains( src.materials, "unit" ) ){
    material.useAlphaTest = 0;
    material.teamColor = &teamColor;
    }

  if( contains( src.materials, "shadow_cast" ) ){
    material.usage.shadowCast = true;
    }

  if( contains( src.materials, "shadowCastTransp" ) ){
    material.usage.shadowCastTransp = true;
    }

  if( contains( src.materials, "glow" ) ){
    material.glow = r.texture( src.name+"/glow" );
    }

  if( contains( src.materials, "terrain.minor" ) ){
    material.usage.terrainMinor = true;
    material.usage.shadowCast   = true;
    }

  if( contains( src.materials, "terrain.main" ) ){
    material.usage.terrainMain = true;
    material.usage.shadowCast  = true;
    }

  if( contains( src.materials, "displace" ) ){
    material.usage.displace = true;
    }

  if( contains( src.materials, "water" ) ){
    material.usage.water = true;
    }

  if( contains( src.materials, "blush" ) ){
    material.usage.blush = true;
    //material.usage.mainPass = !material.usage.mainPassAtst;
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
    material.alphaTrestRef     = 0.1;
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

void Game::loadMission(const std::string &f) {
  GameSerializer s( f );

  if( s.isOpen() )
    serialize(s);
  }

void Game::serialize( GameSerializer &s ) {
  std::string magic = "SAV";
  s + magic;

  if( magic!="SAV")
    return;

  if( s.isReader() ){
    world = 0;
    gui.setupMinimap(0);
    worlds.clear();
    setScenario( new DeatmachScenario(*this, gui, msg) );
    }

  //Tempest::Application::processEvents();

  int plCount = this->plCount()-1;
  s + plCount;
  s + currentPlayer;

  if( s.isReader() ){
    setPlaylersCount( plCount );
    }

  for( int i=0; i<this->plCount(); ++i )
    player(i).serialize(s);

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
      world->setupMaterial.bind( this, &Game::setupMaterials );
      }
    }

  world = worlds[curWorld].get();
  //gui.setupMinimap(*world);

  gui.toogleEditLandMode = Tempest::signal<const Terrain::EditMode&>();
  gui.toogleEditLandMode.bind( *world, &World::toogleEditLandMode );

  gui.paintObjectsHud = Tempest::signal< Tempest::Painter&, int, int>();
  gui.paintObjectsHud.bind( *world, &World::paintHUD );

  if( s.version()>=9 ){
    serializeScenario(s);
    }

  for( size_t i=0; i<worlds.size(); ++i )
    worlds[i]->serialize(s);

  for( int i=0; i<this->plCount(); ++i )
    player(i).computeFog(world);

  if( s.version()<=8 ){
    if( s.version()>=7 ){
      serializeScenario(s);
      } else {
      setScenario( new DeatmachScenario(*this, gui, msg) );
      mscenario->serialize(s);
      }
    }

  updateMissionTargets();
  }

void Game::serializeScenario( GameSerializer &s ) {
  bool isScenario = mscenario->isCampagin();
  s + isScenario;

  std::string taget = mscenario->name();
  if( s.version() >=8 ){
    s + taget;
    } else {
    if( isScenario )
      taget = "mission1";
    }

  if( s.isReader() ){
    if( isScenario ){
      setScenario( ScenarioFactory::create(taget, *this, gui, msg) );
      } else {
      setScenario( new DeatmachScenario(*this, gui, msg) );
      }
    }
  mscenario->serialize(s);
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

void Game::settingsChanged(const GraphicsSettingsWidget::Settings &s) {
  resource.setupSettings(s);
  graphics.setSettings(s);
  }
