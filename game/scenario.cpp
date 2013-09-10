#include "scenario.h"
#include "gui/maingui.h"

#include "game.h"
#include "game/world.h"

#include "behavior/buildingbehavior.h"
#include "util/scenariofactory.h"
#include <cmath>

#include "gui/gen/ui_toppanel.h"
#include "gui/missiontargets.h"

#include "gui/ingamecontrols.h"
#include "gui/scroolwidget.h"
#include "gui/formbuilder.h"
#include "gui/unitinfo.h"
#include "gui/tabwidget.h"
#include "gui/listbox.h"

#include "gui/editterrainpanel.h"
#include "gui/ingamemenu.h"
#include "gui/commandspanel.h"
#include "gui/unitview.h"
#include "gui/unitlist.h"

#include "gui/mainmenu.h"

#include "util/lexicalcast.h"

struct Scenario::AddUnitButton: public Button{
  AddUnitButton( Resource & res, ProtoObject& obj )
      :Button(res), prototype(obj) {
    clicked.bind( *this, &AddUnitButton::click );
    Tempest::Sprite t;
    t = res.pixmap("gui/icon/"+obj.name);
    setText( obj.name );

    icon = t;
    }

  void click(){
    clickedEx( prototype );
    }

  Tempest::signal<const ProtoObject&> clickedEx;
  ProtoObject& prototype;
  };

Scenario::Scenario( Game &game, MainGui &ui, BehaviorMSGQueue & msg )
  :game(game), gui(ui), msg(msg), res(ui.res) {
  acceptMouseObj        = true;
  mouseTracking         = 0;
  selectionRectTracking = 0;
  curMPos               = Tempest::Point( game.w()/2, game.h()/2 );
  lastKEvent            = Tempest::KeyEvent::K_NoKey;

  minimap    = 0;
  mainWidget = 0;
  commands   = 0;
  units      = 0;

  editPanel     = 0;
  settingsPanel = 0;

  gold = 0;
  lim  = 0;

  currPl = 0;
  }

Scenario::~Scenario() {
  setupMinimap(0);
  }

void Scenario::tick() {
  }

void Scenario::uiTick() {
  moveCamera();

  if( gold )
    gold->setText( Lexical::upcast( game.player().gold() ) );
  int freeLim = game.player().limMax()-game.player().lim();

  if( lim )
    lim-> setText( Lexical::upcast( freeLim ) +"/" +
                   Lexical::upcast( game.player().limMax() ) );
  }

const std::vector<Scenario::MissionTaget> &Scenario::tagets() {
  static std::vector<Scenario::MissionTaget> tgNull;
  return tgNull;
  }

void Scenario::mouseDownEvent(Tempest::MouseEvent &e) {
  mouseTracking         = (e.button==Tempest::MouseEvent::ButtonRight);
  if(e.button==Tempest::MouseEvent::ButtonLeft)
    selectionRectTracking = 1;

  gui.selectionRect() = Tempest::Rect(e.x, e.y, 0, 0);
  updateMousePos(e);
  }

void Scenario::mouseUpEvent(Tempest::MouseEvent &e) {
  mouseTracking         = false;
  gui.selectionRect() = Tempest::Rect(-1, -1, 0, 0);
  gui.update();

  int w = game.w(),
      h = game.h();

  World  &world  = game.curWorld();
  Player &player = game.player();

  F3 v = unProject( e.x, e.y );

  if( player.editObj==0 ){
    if( e.button==Tempest::MouseEvent::ButtonLeft ){
      if( selectionRectTracking==2 )
        world.updateSelectionFlag ( msg, player.number() ); else
        world.updateSelectionClick( msg, player.number(), e.x, e.y,
                                    w, h );
      }

    if( e.button==Tempest::MouseEvent::ButtonRight ){
      size_t obj = world.unitUnderMouse( e.x, e.y,
                                         w, h );

      if( obj==size_t(-1) ){
        game.message( player.number(),
                      AbstractBehavior::Move,
                      World::coordCastD(v.data[0]),
                      World::coordCastD(v.data[1]) );
        } else {
        msg.message_st( player.number(),
                        AbstractBehavior::ToUnit,
                        obj );
        }
      }
    }

  selectionRectTracking = false;
  updateMousePos(e);

  if( e.button==Tempest::MouseEvent::ButtonLeft && player.editObj ){
    msg.message( player.number(), Behavior::EditNext );
    }

  if( e.button==Tempest::MouseEvent::ButtonRight && player.editObj ){
    msg.message( player.number(), Behavior::EditDel );
    }

  world.clickEvent( World::coordCastD(v.data[0]),
                    World::coordCastD(v.data[1]),
                    e );
  }

void Scenario::mouseMoveEvent( Tempest::MouseEvent &e ) {
  World  &world  = game.curWorld();
  curMPos = Tempest::Point(e.x, e.y);

  int w = game.w(),
      h = game.h();

  if( !selectionRectTracking && gui.mouseMoveEvent(e) ){
    acceptMouseObj = false;
    return;
    }

  acceptMouseObj = true;

  if( selectionRectTracking ){
    selectionRectTracking = 2;

    Tempest::Rect & r = gui.selectionRect();
    r.w = e.x - gui.selectionRect().x;
    r.h = e.y - gui.selectionRect().y;

    //gui.update();

    double ww = w/2.0, hh = h/2.0;
    world.updateMouseOverFlag(    r.x/ww - 1.0,
                                -(r.y/hh - 1.0),
                                 (r.x+r.w)/ww - 1.0,
                               -((r.y+r.h)/hh - 1.0) );
    }

  updateMousePos(e);
  }

void Scenario::mouseWheelEvent(Tempest::MouseEvent &e) {
  World  & world  = game.curWorld();
  Player & player = game.player();

  if( (player.editObj && !game.isReplayMode() ) &&
      lastKEvent!=Tempest::KeyEvent::K_Down ){
    int dR = 10;
    if( e.delta<0 )
      dR = -10;

    msg.message( player.number(), Behavior::EditRotate, dR, 0 );
    } else {
    if( e.delta<0 )
      world.camera.setDistance( world.camera.distance() * 1.1 ); else
      world.camera.setDistance( world.camera.distance() / 1.1 );
    }
  }

void Scenario::keyDownEvent(Tempest::KeyEvent &e) {
  lastKEvent = e.key;
  }

void Scenario::keyUpEvent(Tempest::KeyEvent &) {
  lastKEvent = Tempest::KeyEvent::K_NoKey;
  }

bool Scenario::isSelectionRectTracking() const {
  return selectionRectTracking;
  }

void Scenario::updateMousePos(Tempest::MouseEvent &e) {
  World  &world  = game.curWorld();
  Player &player = game.player();

  int w = game.w(),
      h = game.h();

  curMPos = e.pos();

  F3 v = unProject( curMPos.x, curMPos.y );
  int vx = World::coordCastD(v.data[0])/Terrain::quadSize,
      vy = World::coordCastD(v.data[1])/Terrain::quadSize;

  world.setMousePos( vx*Terrain::quadSize,
                     vy*Terrain::quadSize,
                     World::coordCastD(v.data[2]) );
  if( acceptMouseObj ){
    world.setMouseObject( world.unitUnderMouse( curMPos.x,
                                                curMPos.y,
                                                w, h ) );
    } else {
    world.setMouseObject( -1 );
    }

  if( player.editObj ){
    int x = World::coordCastD(v.data[0]),
        y = World::coordCastD(v.data[1]);

    if( player.editObj &&
        player.editObj->behavior.find<BuildingBehavior>() ){
      x = (x/Terrain::quadSize)*Terrain::quadSize;
      y = (y/Terrain::quadSize)*Terrain::quadSize;
      }

    msg.message( player.number(),
                 Behavior::EditMove,
                 x, y );
    }
  }

F3 Scenario::unProject( int x, int y, float destZ ) {
  World  &world  = game.curWorld();
  int w = game.w(),
      h = game.h();

  Tempest::Matrix4x4 mat = world.camera.projective();
  mat.mul( world.camera.view() );
  mat.inverse();

  float px =  2.0*(x-w/2.0)/double( w ),
        py = -2.0*(y-h/2.0)/double( h );

  float vec1[4], vec2[4];
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

  float k = (vec1[2]-destZ)/vec2[2];
  for( int i=0; i<4; ++i ){
    vec1[i] -= k*vec2[i];
    }


  F3 r;
  std::copy( vec1, vec1+3, r.data );
  return r;
  }

F3 Scenario::unProject(int x, int y) {
  float dummy;
  return unProjectRz(x,y, dummy);
  }

F3 Scenario::unProjectRz(int x, int y, float &rz) {
  World  &world  = game.curWorld();

  float min = -2 + world.camera.z(),
        max =  2 + world.camera.z();
  F3 ret = unProject( x, y, 0 );
  rz = 0;

  float err = fabs( world.zAt(ret.data[0], ret.data[1]) - ret.data[2] );

  for( int i=0; i<20; ++i ){
    float z = min+(max-min)*i/20.0;
    F3 v = unProject( x, y, z );
    float err2 = fabs( world.zAt(v.data[0], v.data[1]) - v.data[2] );

    if( err2<err ){
      err = err2;
      ret = v;
      rz = z;
      }
    }

  return ret;
  }

F3 Scenario::project(float x, float y, float z) {
  World  &world  = game.curWorld();

  Tempest::Matrix4x4 mat = world.camera.projective();
  mat.mul( world.camera.view() );
  //mat.transpose();

  F3 out;
  float data[4];
  mat.project( x,y,z, 1, data[0], data[1], data[2], data[3]);

  for( int i=0; i<4; ++i )
    data[i] /= data[3];

  std::copy( data, data+3, out.data );

  return out;
  }

void Scenario::moveCamera() {
  World  &world  = game.curWorld();
  int w = game.w(),
      h = game.h();

  if( isSelectionRectTracking() )
    return;

#ifndef __ANDROID__
  const double cameraStep = 0.1;
  const int sensetive = 20;

  if( curMPos.x < sensetive || lastKEvent==Tempest::KeyEvent::K_Left ){
    world.moveCamera( -cameraStep, 0 );
    }
  if( curMPos.x > w - sensetive || lastKEvent==Tempest::KeyEvent::K_Right ){
    world.moveCamera( cameraStep, 0 );
    }

  if( curMPos.y < sensetive || lastKEvent==Tempest::KeyEvent::K_Up ){
    world.moveCamera( 0, -cameraStep );
    }
  if( curMPos.y > h - sensetive || lastKEvent==Tempest::KeyEvent::K_Down ){
    world.moveCamera( 0, cameraStep );
    }
#endif
  }

void Scenario::setCurrPl(size_t i) {
  currPl = i;
  if( mainWidget )
    mainWidget->onSetPlayer(i);
  }

void Scenario::addEditorObject( const ProtoObject &p ) {
  if( mainWidget )
    mainWidget->addObject( p, currPl );
  }

void Scenario::setupUI( InGameControls *mw,
                        Resource & res ) {
  using namespace Tempest;

  mainWidget = mw;
  currPl     = 1;

  showEditPanel = Shortcut(mainWidget, Tempest::KeyEvent::K_F9);
  showSettings  = Shortcut(mainWidget, Tempest::KeyEvent::K_F8);

  mainWidget->setLayout( Tempest::Vertical );
  mainWidget->layout().setMargin( Tempest::Margin(4) );
  mainWidget->useScissor( false );

  Widget * top = new Widget(),
         * cen = new Widget();

  { SizePolicy p;
    p.typeV = Tempest::Expanding;
    cen->setSizePolicy(p);
    }

  mainWidget->layout().add( top );
  mainWidget->layout().add( cen );

  mainWidget->layout().add( createConsole(mainWidget, msg) );

  top->setLayout( Tempest::Horizontal );

  SizePolicy p;
  p.maxSize.h = 30;
  p.typeV = Tempest::FixedMax;

  {
    UI::TopPanel p;
    p.setupUi( top, res );
    //p.fullScr->clicked.bind( toogleFullScreen );
    p.menu->clicked.bind( *this, &Scenario::showMenu );
    p.menu->setShortcut( Tempest::Shortcut(p.menu, Tempest::KeyEvent::K_ESCAPE) );

    p.frmEdit->clicked.bind( *this, &Scenario::showFormBuilder );
    gold = p.gold;
    lim  = p.lim;

    gold->icon = res.pixmap("gui/icon/gold");
    lim-> icon = res.pixmap("gui/icon/house");

    gold->setHint(L"$(gold)");
    lim ->setHint(L"$(units_limit)");

    p.frmEdit->setVisible(0);
    p.fullScr->setVisible(0);
    }

  top->setSizePolicy(p);

  editPanel     = createEditPanel(mainWidget, res);
  settingsPanel = createSettingsPanel(mainWidget, res);

  cen->setLayout( Tempest::Horizontal );
  cen->layout().add( new MissionTargets(game, res) );

  Tempest::Widget * box = new Tempest::Widget();
  box->layout().add( editPanel );
  box->layout().add( settingsPanel );
  cen->layout().add( box );

  cen->useScissor( false );
  box->useScissor( false );

  //editPanel->setVisible(0);
  //settingsPanel->setVisible(0);

  showEditPanel.activated.bind( *this, &Scenario::toogleEditPanel );
  showSettings. activated.bind( *this, &Scenario::toogleSettingsPanel );
  }

void Scenario::setupMinimap(World * w) {
  if( minimap )
    minimap->setup(w);
  }

void Scenario::renderMinimap() {
  if( minimap )
    minimap->render();
  }

void Scenario::onUnitDied(GameObject &obj) {
  if( units )
    units->onUnitDied(obj);
  }

void Scenario::updateSelectUnits(const std::vector<GameObject *> &u) {
  if( units )
    units->setup(u);
  }

std::string Scenario::name() const {
  return ScenarioFactory::productName(this);
  }

void Scenario::showFormBuilder() {
  new FormBuilder(res, mainWidget);
  }

void Scenario::toogleEditPanel() {
  editPanel->setVisible( !editPanel->isVisible() );
  }

void Scenario::toogleSettingsPanel() {
  settingsPanel->setVisible( !settingsPanel->isVisible() );
  }

Tempest::Widget *Scenario::createSettingsPanel( InGameControls *mainWidget,
                                                Resource & res ) {
  GraphicsSettingsWidget * s = new GraphicsSettingsWidget(res);
  s->onSettingsChanged.bind( mainWidget->onSettingsChanged );
  return s;
  }

EditTerrainPanel *Scenario::createLandEdit( InGameControls *mainWidget,
                                            Resource & res ) {
  EditTerrainPanel* p = new EditTerrainPanel(res, game.prototypes());
  p->toogleEditLandMode.bind( mainWidget->toogleEditLandMode );

  return p;
  }

Tempest::Widget *Scenario::createConsole( InGameControls *mainWidget,
                                          BehaviorMSGQueue & q ) {
  using namespace Tempest;

  Tempest::Widget* console = new Widget();
  console->setMaximumSize( SizePolicy::maxWidgetSize().w, 220 );
  console->setMinimumSize( 0, 220);

  SizePolicy p;
  p.minSize = Tempest::Size(220, 220);
  p.maxSize = Tempest::Size(220, 220);

  console->setLayout( Tempest::Horizontal );
  console->layout().setSpacing(6);

  Panel * img = new Panel( res );
  img->setSizePolicy(p);

  minimap = new MiniMapView(res);
  minimap->setTexture( res.texture("grass/diff") );
  minimap->mouseEvent.bind( mainWidget->minimapEvent );

  img->setLayout( Vertical );
  img->layout().add( minimap );
  img->layout().setMargin(8);

  console->layout().add( img );

  Widget * cen = new Widget();
  cen->setLayout( Tempest::Horizontal );

  Panel * cenp = new Panel( res );
  cenp->setMaximumSize( SizePolicy::maxWidgetSize().w, 220 );
  cenp->layout().setMargin(7);

  commands = new CommandsPanel(res, q);
  UnitInfo *uinfo = new UnitInfo(res);
  UnitView *uview = new UnitView(res);
  units = new UnitList(commands,res, uview, uinfo);
  units->setCameraPos.bind( mainWidget->setCameraPos );

  //cenp->layout().add( new Tempest::Widget() );
  cenp->layout().add( units );
  cenp->layout().add( uinfo );

  cen->setSizePolicy( Tempest::Expanding );

  Tempest::Margin m(0);
  m.top = 50;
  cen->layout().setMargin(m);

  cen->layout().add( cenp );

  { Panel * avatar = new Panel(res);
    SizePolicy pa;
    pa.typeH = Tempest::FixedMin;
    pa.minSize.w = 120;
    avatar->setSizePolicy( pa );

    uview->renderScene.bind( mainWidget->renderScene );
    mainWidget->updateView.bind( *uview, &UnitView::updateView );

    avatar->layout().add( uview );
    avatar->layout().setMargin(8);

    cen->layout().add( avatar );
    }

  console->layout().add( cen );

  commands->onPageCanged.bind( *mainWidget, &InGameControls::removeAllHooks );

  commands->setSizePolicy( img->sizePolicy() );
  commands->setMinimumSize(270, 220);
  commands->setMaximumSize(270, 220);
  console->layout().add( commands );

  return console;
  }

Tempest::Widget *Scenario::createEditPanel( InGameControls *mainWidget, Resource &res ) {
  using namespace Tempest;

  TabWidget *tabs = new TabWidget(res);
  tabs->resize( 200, 400 );

  Widget *p = new Widget();
  tabs->addTab(p);
  EditTerrainPanel *eterr = createLandEdit(mainWidget, res);
  tabs->onTabChanged.bind( *eterr, &EditTerrainPanel::disableEdit );
  tabs->onTabChanged.bind( mainWidget->toogleEditTab );
  tabs->addTab( eterr );

  tabs->setDragable(1);
  p->layout().setMargin( 6 );
  p->setLayout( Tempest::Vertical );

  ScroolWidget *w = new ScroolWidget(res);
  //w->useScissor(0);

  auto proto = game.prototypes().allClasses();

  for( auto i=proto.begin(); i!=proto.end(); ++i ){
    bool ok = false;
    for( size_t r=0; !ok && r<(**i).view.size(); ++r )
      if( (**i).view[r].name.size() && !(**i).isLandTile() )
        ok = true;

    if( ok ){
      AddUnitButton *b = new AddUnitButton(res, **i);
      b->clickedEx.bind( *this, &Scenario::addEditorObject );

      SizePolicy p = b->sizePolicy();
      p.typeH = Tempest::Preferred;
      b->setSizePolicy(p);

      w->centralWidget().layout().add( b );
      }
    }

  p->layout().add(w);
  ListBox *lbox = new ListBox(res);

  std::vector<std::wstring> items;
  for( int i=0; i<8; ++i ){
    std::wstringstream str;
    str << i;
    items.push_back( str.str() );
    }
  lbox->setItemList(items);
  lbox->onItemSelected.bind( *this, &Scenario::setCurrPl );
  lbox->setCurrentItem(currPl);

  p->layout().add( lbox );

  //tabs->setVisible(0);
  return tabs;
  }

void Scenario::showMenu() {
  game.pause(1);

  InGameMenu *m = new InGameMenu(res, mainWidget);

  m->save.bind( mainWidget->save );
  m->load.bind( mainWidget->load );
  m->onClosed.bind( game, &Game::unsetPause );
  m->quit.bind( game.exitGame );
  }

void Scenario::showMainMenu() {
  game.pause(1);

  MainMenu *m = new MainMenu(res, mainWidget);
  m->onClosed.bind( game, &Game::unsetPause );
  }

Player *Scenario::createPlayer() {
  return new Player( plCount() );
  }

void Scenario::addPlayer() {
  players.push_back( std::shared_ptr<Player>( createPlayer() ) );

  if( players.size() == 2 ){
    players[1]->setHostCtrl(1);
    }

  players.back()->onUnitSelected.bind( game, &Game::onUnitsSelected );
  players.back()->onUnitDied    .bind( game, &Game::onUnitDied );
  }

size_t Scenario::plCount() const {
  return players.size();
  }

void Scenario::setPlaylersCount(int c) {
  players.clear();

  ++c;
  for( int i=0; i<c; ++i )
    addPlayer();
  }

Player &Scenario::player(int i) {
  while( int(players.size())<=i )
    addPlayer();

  return *players[i];
  }

Player &Scenario::player() {
  return game.player();
  }
