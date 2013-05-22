#include "desertstrikescenario.h"

#include "game.h"
#include "gui/ingamecontrols.h"
#include "gui/gen/ui_toppanel.h"
#include "gui/missiontargets.h"

#include "gui/unitinfo.h"
#include "gui/minimapview.h"

#include "util/bytearrayserialize.h"
#include <sstream>
#include "gui/lineedit.h"

struct DesertStrikeScenario::BuyButton: public Button {
  BuyButton( Resource & r,
             const ProtoObject& obj,
             PlInfo & pl ):Button(r), p(obj), pl(pl){
    setMinimumSize( 50, 50 );
    setMaximumSize( 50, 50 );

    Texture t;
    t.data = res.pixmap("gui/icon/"+obj.name);
    //setText( obj.name );

    icon = t;
    font = Font(15);

    clicked.bind(*this, &BuyButton::emitClick);
    }

  void paintEvent(Tempest::PaintEvent &e){
    Button::paintEvent(e);

    Tempest::Painter p(e);
    p.setFont(font);

    std::stringstream s;
    s << pl.units[this->p.name];
    p.drawText(3,3, s.str());
    }

  void emitClick(){
    onClick(p);
    }

  Tempest::signal<const ProtoObject&> onClick;
  const ProtoObject& p;
  PlInfo & pl;
  Font font;
  };

struct DesertStrikeScenario::Minimap: MiniMapView{
  struct BuyButton: public Button {
    BuyButton( Resource & r ):Button(r) {
      setMinimumSize( 50, 50 );
      setMaximumSize( 50, 50 );

      Texture t;
      t.data = res.pixmap("gui/icon/gold");
      //setText( obj.name );

      icon = t;
      }
    };

  Minimap( Resource &res, Game & game ):MiniMapView(res), game(game){
    using namespace Tempest;

    base = new Widget();
    base->setLayout( Vertical );

    Widget* w = new Widget();
    w->setLayout( Horizontal );
    BuyButton *btn = new BuyButton(res);
    btn->clicked.bind(*this, &Minimap::buy );
    btn->icon.data = res.pixmap("gui/icon/atack");
    w->layout().add( btn );

    ledit = new LineEdit(res);
    w->layout().add( ledit );

    btn = new BuyButton(res);
    btn->clicked.bind(*this, &Minimap::sell );
    w->layout().add( btn );

    w->setMaximumSize( w->sizePolicy().maxSize.w, 50 );

    base->layout().add(w);
    w = new Widget();
    //w->setSizePolicy( Expanding );
    base->layout().add( w );

    setLayout( Vertical );
    layout().add(base);

    ledit->setText(L"128");
    ledit->setEditable(0);

    //setFocusPolicy();
    base->setVisible(0);
    }

  void paintEvent(Tempest::PaintEvent &e){
    MiniMapView::paintEvent(e);
    paintNested(e);
    }

  void mouseDownEvent(Tempest::MouseEvent &){
    base->setVisible( 0 );
    }

  void setupUnit( const std::string & unit ){
    unitToBuy = unit;

    base->setVisible(1);
    base->setFocus(1);
    }

  void hideInfo( bool f ){
    base->setVisible( base->hasChildFocus() || f );
    }

  void buy(){
    std::vector<char> data;
    ByteArraySerialize s(data, ByteArraySerialize::Write);

    s.write(game.player().number()-1);
    s.write( unitToBuy );
    s.write( 'b' );
    game.message( data );
    }

  void sell(){
    std::vector<char> data;
    ByteArraySerialize s(data, ByteArraySerialize::Write);

    s.write(game.player().number()-1);
    s.write( unitToBuy );
    s.write( 's' );
    game.message( data );
    }

  Widget* base;
  LineEdit * ledit;
  std::string unitToBuy;

  Game &game;
  };

struct DesertStrikeScenario::BuyUnitPanel: public Tempest::Widget {
  BuyUnitPanel( Resource & res,
                Game & game,
                PlInfo & pl,
                Minimap * mmap): res(res), game(game), mmap(mmap) {
    using namespace Tempest;
    setMinimumSize(250, 200);
    setMaximumSize(250, 200);

    frame.data = res.pixmap("gui/hintFrame");
    setLayout( Vertical );
    layout().setMargin(15);

    const char * pr[3][4] = {
      {"gelion", "pikeman", "hummer", "fire_mage"},
      {"fire_mage", "balista"},
      {"fire_element", "golem"}
      };

    for( int i=0; i<3; ++i ){
      Widget* l = new Widget();
      l->setLayout( Horizontal );
      for( int r=0; r<4; ++r ){
        if( pr[i][r] ){
          const ProtoObject & obj = game.prototype(pr[i][r]);

          DesertStrikeScenario::BuyButton * u = new DesertStrikeScenario::BuyButton(res, obj, pl);
          u->onClick.bind( *this, &BuyUnitPanel::onUnit );
          l->layout().add( u );
          }
        }
      layout().add(l);
      }
    }

  void paintEvent(Tempest::PaintEvent &e){
    Tempest::Painter p(e);

    MainGui::drawFrame(p, frame, Tempest::Point(), size() );

    paintNested(e);
    }

  void onUnit( const ProtoObject & p ){
    setupBuyPanel(p.name);
    }

  void setupBuyPanel( const std::string & s ){
    mmap->setupUnit(s);
    }

  Resource  & res;
  Tempest::Bind::UserTexture frame;
  Game &game;

  Minimap * mmap;
  };

DesertStrikeScenario::DesertStrikeScenario(Game &g, MainGui &ui, BehaviorMSGQueue &msg)
  :Scenario(g,ui, msg){
  tNum     = 0;
  interval = 1000;
  isMouseTracking = false;

  plC[0].units["pikeman"] = 3;
  plC[1].units["pikeman"] = 3;

  gui.update();
  }

void DesertStrikeScenario::mouseDownEvent( Tempest::MouseEvent &e ) {
  mpos3d          = unProject( e.x, e.y );
  isMouseTracking = true;
  }

void DesertStrikeScenario::mouseUpEvent( Tempest::MouseEvent & ) {
  isMouseTracking = false;
  }

void DesertStrikeScenario::mouseMoveEvent( Tempest::MouseEvent &e ) {
  if( !isMouseTracking && gui.mouseMoveEvent(e) ){
    acceptMouseObj = false;
    return;
    }

  if( isMouseTracking ){
    F3 m = unProject( e.x, e.y );
    game.curWorld().moveCamera( mpos3d.data[0]-m.data[0],
                                mpos3d.data[1]-m.data[1]);
    mpos3d = unProject( e.x, e.y );
    }
  }

void DesertStrikeScenario::customEvent(const std::vector<char> &m) {
  ByteArraySerialize s( m );

  int pl;
  char ch;
  std::string name;

  s.read(pl);
  s.read(name);
  s.read(ch);

  if( ch=='b' ){
    ++plC[ pl ].units[name];

    //debug
    ++plC[ 1 ].units[name];
    } else
  if( ch=='s' && plC[ pl ].units[name]>0 ){
    --plC[ pl ].units[name];

    //debug
    --plC[ 1 ].units[name];
    }
  }

void DesertStrikeScenario::onUnitDied(GameObject &obj) {
  if( obj.playerNum()>=1 && obj.playerNum()<=2 )
    --plC[obj.playerNum()-1].realCount[ obj.getClass().name ];
  }

void DesertStrikeScenario::tick() {
  int w = game.curWorld().terrain().width() *Terrain::quadSize;
  int h = game.curWorld().terrain().height()*Terrain::quadSize;

  if( tNum%interval==0 ){
    mkUnits( 1,
             10*Terrain::quadSize, 10*Terrain::quadSize,
             w-10*Terrain::quadSize, h-10*Terrain::quadSize);
    }

  if( tNum%interval==0 ){
    mkUnits( 2,
             w-10*Terrain::quadSize, h-10*Terrain::quadSize,
             10*Terrain::quadSize, 10*Terrain::quadSize );
    }

  ++tNum;
  if( tNum/interval>=2 ){
    tNum = 0;
    }
  }

void DesertStrikeScenario::mkUnits( int pl,
                                    int   x, int   y,
                                    int tgX, int tgY ){
  --pl;

  std::map<std::string, int>::iterator u, e = plC[pl].units.end();

  for( u = plC[pl].units.begin(); u!=e; ++u ){
    int& c = plC[pl].realCount[u->first];
    for( int i=0; i<u->second; ++i ){
      if( c < u->second ){
        GameObject& obj = game.curWorld().addObject(u->first, pl+1);
        ++c;

        obj.setPosition( x, y );
        obj.behavior.message( Behavior::MoveSingle, tgX, tgY );
        obj.behavior.message( Behavior::AtackMove,  tgX, tgY );
        }
      }
    }
  }

void DesertStrikeScenario::onStartGame() {
  {
    GameObject& obj = game.curWorld().addObject("tower", 1);
    int w = game.curWorld().terrain().width() *Terrain::quadSize;
    int h = game.curWorld().terrain().height()*Terrain::quadSize;

    int p = 40-9;
    obj.setPosition( p*w/80, p*h/80 );
    }

  {
    GameObject& obj = game.curWorld().addObject("tower", 2);
    int w = game.curWorld().terrain().width() *Terrain::quadSize;
    int h = game.curWorld().terrain().height()*Terrain::quadSize;

    int p = 40+8;
    obj.setPosition( p*w/80, p*h/80 );
    }
  }

void DesertStrikeScenario::setupUI( InGameControls *mw, Resource &res ) {
  using namespace Tempest;

  mainWidget = mw;
  //currPl     = 1;

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
    p.menu->clicked.bind( *this, &DesertStrikeScenario::showMenu );
    p.menu->setShortcut( Tempest::Shortcut(p.menu, Tempest::KeyEvent::K_ESCAPE) );

    p.frmEdit->clicked.bind( *this, &DesertStrikeScenario::showFormBuilder );
    gold = p.gold;
    lim  = p.lim;

    gold->icon.data = res.pixmap("gui/icon/gold");
    lim-> icon.data = res.pixmap("gui/icon/house");

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

  showEditPanel.activated.bind( *this, &DesertStrikeScenario::toogleEditPanel );
  showSettings. activated.bind( *this, &DesertStrikeScenario::toogleSettingsPanel );
  }

Tempest::Widget *DesertStrikeScenario::createConsole( InGameControls *mainWidget,
                                                      BehaviorMSGQueue &  ) {
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

  mmap = new Minimap(res,game);
  minimap = mmap;
  minimap->setTexture( res.texture("grass/diff") );
  minimap->mouseEvent.bind( mainWidget->minimapEvent );

  img->setLayout( Vertical );
  img->layout().add( minimap );
  img->layout().setMargin(8);

  console->layout().add( img );
  Widget *w = new Widget();
  w->setSizePolicy( Tempest::Expanding );
  console->layout().add( w );

  BuyUnitPanel * right = new BuyUnitPanel( res, game, plC[0], mmap );
  console->layout().add( right );
  console->useScissor(false);

  return console;
  }
