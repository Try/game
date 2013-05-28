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
#include "gui/unitview.h"
#include "gui/richtext.h"
#include "gui/listbox.h"

#include "lang/lang.h"

#include "util/math.h"

struct DesertStrikeScenario::NumButton: public Button {
  NumButton( Resource & r ):Button(r) {
    setMinimumSize( 50, 50 );
    setMaximumSize( 50, 50 );

    numFrame.data = res.pixmap("gui/hintFrame");
    num = 0;
    }

  void paintEvent(Tempest::PaintEvent &e){
    using namespace Tempest;

    Button::paintEvent(e);

    Painter p(e);
    p.setFont(font);

    std::stringstream s;
    s << num;

    p.setTexture(numFrame);
    Size tsz = font.textSize(res, s.str());
    int nw = numFrame.data.rect.w,
        th  = tsz.h+10,
        th2 = tsz.h+6;

    if( tsz.w+11-nw/2 > 0 )
      p.drawRect( Rect( 4, h()-th, tsz.w+11-nw/2, th2 ),
                  Rect( nw/2, 0, 1, th2) );

    p.drawRect( Rect( 4+std::max( tsz.w+11-nw/2, 0 ), h()-th,
                      nw/2, th2 ),
                Rect( nw-nw/2, 0, nw/2, th2) );

    p.setFont( font );
    p.drawText(4, h()-tsz.h-7, s.str());
    }

  int  num;
  Font font;
  Texture numFrame;
  };


struct DesertStrikeScenario::BuyButton: public NumButton {
  BuyButton( Resource & r,
             const ProtoObject& obj,
             PlInfo & pl ):NumButton(r), p(obj), pl(pl){
    icon.data        = res.pixmap("gui/icon/"+obj.name);
    //setText( obj.name );

    font = Font(15);

    clicked.bind(*this, &BuyButton::emitClick);
    }

  void emitClick(){
    onClick(p);
    }

  void paintEvent(Tempest::PaintEvent &e){
    num = pl.units[this->p.name];
    NumButton::paintEvent(e);
    }

  Tempest::signal<const ProtoObject&> onClick;

  const ProtoObject& p;
  PlInfo & pl;
  };

struct DesertStrikeScenario::GradeButton: public ListBox {
  GradeButton( Resource & r,
               PlInfo & p,
               const std::string& obj,
               const std::string& t ):ListBox(r), type(t), pl(p){
    icon.data        = res.pixmap(obj);
    //setText( obj.name );

    setMinimumSize( 50, 50 );
    setMaximumSize( 50, 50 );
    font = Font(15);

    clicked.bind(*this, &GradeButton::emitClick);
    }

  void emitClick(){
    //onClick(p);
    }

  void paintEvent(Tempest::PaintEvent &e){
    //num = pl.getParam(type);
    ListBox::paintEvent(e);
    }

  //Tempest::signal<const ProtoObject&> onClick;
  std::string type;
  PlInfo & pl;
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

  Minimap( Resource &res,
           Game & game,
           PlInfo & pl ):MiniMapView(res), game(game), pl(pl){
    using namespace Tempest;

    base = new UnitView(res);
    base->setLayout( Vertical );

    Widget* w = new Widget();
    w->setLayout( Horizontal );

    BuyButton *btn = new BuyButton(res);
    btn->clicked.bind(*this, &Minimap::sell );
    w->layout().add( btn );

    ledit = new RichText(res);
    w->layout().add( ledit );

    btn = new BuyButton(res);
    btn->clicked.bind(*this, &Minimap::buy );
    btn->icon.data = res.pixmap("gui/icon/atack");
    w->layout().add( btn );

    w->setMaximumSize( w->sizePolicy().maxSize.w, 50 );
    w->setSizePolicy( Preferred, FixedMax );

    base->layout().add(w);
    w = new Widget();
    //w->setSizePolicy( Expanding );
    base->layout().add( w );

    const char* icon[] = {
      "gui/icon/atack",
      "gui/item/shield",
      "gui/icon/atack"
    };

    for( int i=0; i<2; ++i ){
      w = new Widget();
      w->setMaximumSize( w->sizePolicy().maxSize.w, 25 );
      w->setSizePolicy( Preferred, FixedMax );

      w->setLayout( Horizontal );
      RichText * cost = new RichText(res);
      cost->setText(L"<s>123</s>");

      BuyButton * btn = new BuyButton(res);
      btn->setMaximumSize(25);
      btn->icon.data = res.pixmap(icon[i]);

      w->layout().add( btn );
      w->layout().add( cost );
      base->layout().add( w );

      info[i] = cost;
      }

    setLayout( Vertical );
    layout().add(base);

    //ledit->setEditable(0);

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
    base->setupUnit(game, unit);

    updateValues();
    }

  void updateValues(){
    { std::wstringstream s;
      std::string name = "$(" +unitToBuy+")";
      s << Lang::tr(name) << std::endl << L" - " << game.prototype(unitToBuy).data.gold;
      ledit->setText( s.str() );
      }

    { std::wstringstream s;
      s << game.prototype(unitToBuy).data.atk[0].damage <<" / "
        << game.prototype(unitToBuy).data.atk[0].range;
      info[0]->setText( s.str() );
      }

    { std::wstringstream s;
      s << game.prototype(unitToBuy).data.armor;
      info[1]->setText( s.str() );
      }
    }

  void hideInfo(){
    base->setVisible( 0 );//base->hasChildFocus() );
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

  UnitView* base;
  RichText * ledit, *info[2];
  std::string unitToBuy;

  Game &game;
  PlInfo & pl;
  };

struct DesertStrikeScenario::TranscurentPanel: public Tempest::Widget {
  TranscurentPanel( Resource & res ):res(res){
    frame.data = res.pixmap("gui/hintFrame");
    }

  void paintEvent(Tempest::PaintEvent &e){
    Tempest::Painter p(e);

    MainGui::drawFrame(p, frame, Tempest::Point(), size() );

    paintNested(e);
    }

  Resource  & res;
  Tempest::Bind::UserTexture frame;
  };

struct DesertStrikeScenario::UpgradePanel: public TranscurentPanel {
  UpgradePanel( Resource & res,
                Game & /*game*/,
                PlInfo & pl ):TranscurentPanel(res){
    using namespace Tempest;

    setMinimumSize(75, 200);
    setMaximumSize(75, 200);
    layout().setMargin(15);

    setSizePolicy( FixedMin );
    setLayout( Vertical );

    DesertStrikeScenario::GradeButton * u = 0;
    u = new DesertStrikeScenario::GradeButton(res, pl, "gui/icon/castle", "castle" );
    layout().add( u );

    u = new DesertStrikeScenario::GradeButton(res, pl, "gui/icon/atack", "atack" );
    layout().add( u );

    u = new DesertStrikeScenario::GradeButton(res, pl, "gui/item/shield", "armor" );
    layout().add( u );
    }

  };

struct DesertStrikeScenario::SpellPanel: public TranscurentPanel {
  SpellPanel( Resource & res,
              Game & /*game*/,
              PlInfo & pl ):TranscurentPanel(res){
    using namespace Tempest;

    setMinimumSize(75, 200);
    setMaximumSize(75, 200);
    layout().setMargin(15);

    setSizePolicy( FixedMin );
    setLayout( Vertical );

    DesertStrikeScenario::GradeButton * u = 0;
    u = new DesertStrikeScenario::GradeButton(res, pl, "gui/icon/fire_strike", "fire_strike" );
    layout().add( u );

    u = new DesertStrikeScenario::GradeButton(res, pl, "gui/icon/blink", "blink" );
    layout().add( u );
    }

  };

struct DesertStrikeScenario::BuyUnitPanel: public TranscurentPanel {
  BuyUnitPanel( Resource & res,
                Game & game,
                PlInfo & pl,
                Minimap * mmap): TranscurentPanel(res), game(game), mmap(mmap) {
    using namespace Tempest;
    setMinimumSize(250, 200);
    setMaximumSize(250, 200);

    setLayout( Vertical );
    layout().setMargin(15);

    const char * pr[3][4] = {
      {"gelion", "pikeman", "incvisitor", "water_mage"},
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

  void onUnit( const ProtoObject & p ){
    setupBuyPanel(p.name);
    }

  void setupBuyPanel( const std::string & s ){
    mmap->setupUnit(s);
    }

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
  g.prototypes().load("campagin/td.xml");
  }

DesertStrikeScenario::~DesertStrikeScenario() {
  game.prototypes().unload();
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
    int g = game.prototype(name).data.gold;
    if( game.player(pl+1).gold() >= g ){
      ++plC[ pl ].units[name];
      game.player(pl+1).setGold( game.player(pl+1).gold() - game.prototype(name).data.gold );

      //debug
      ++plC[ 1 ].units[name];
      }
    } else
  if( ch=='s' && plC[ pl ].units[name]>0 ){
    --plC[ pl ].units[name];

    //debug
    --plC[ 1 ].units[name];
    }

  mmap->updateValues();
  }

void DesertStrikeScenario::onUnitDied(GameObject &obj) {
  if( obj.playerNum()>=1 && obj.playerNum()<=2 )
    --plC[obj.playerNum()-1].realCount[ obj.getClass().name ];
  }

void DesertStrikeScenario::tick() {
  int w = game.curWorld().terrain().width() *Terrain::quadSize;
  int h = game.curWorld().terrain().height()*Terrain::quadSize;

  int p1 = 10,
      p2 = 80-p1;
  if( tNum%interval==0 ){
    mkUnits( 1,
             p1*w/80, p1*h/80,
             p2*w/80, p2*h/80 );
    }

  if( tNum%interval==0 ){
    mkUnits( 2,
             p2*w/80, p2*h/80,
             p1*w/80, p1*h/80);
    }

  ++tNum;
  if( tNum/interval>=2 ){
    tNum = 0;
    }

  for( size_t i=1; i<game.plCount(); ++i )
    ;//game.player(i).addGold(1);
  }

void DesertStrikeScenario::mkUnits( int pl,
                                    int   x, int   y,
                                    int tgX, int tgY ){
  --pl;

  std::map<std::string, int>::iterator u, e = plC[pl].units.end();
  int count = 0;

  for( u = plC[pl].units.begin(); u!=e; ++u )
    count += u->second;

  int qc = Math::sqrt(count)+1;

  int id = 0;
  for( u = plC[pl].units.begin(); u!=e; ++u ){
    int& c = plC[pl].realCount[u->first];
    for( int i=0; i<u->second; ++i ){
      if( c < 2*u->second ){
        GameObject& obj = game.curWorld().addObject(u->first, pl+1);
        ++c;
        ++id;

        obj.setPosition( x+id%qc, y+id/qc );
        obj.behavior.message( Behavior::MoveSingle, tgX, tgY );
        obj.behavior.message( Behavior::AtackMove,  tgX, tgY );
        }
      }
    }
  }

void DesertStrikeScenario::onStartGame() {
  int w = game.curWorld().terrain().width() *Terrain::quadSize;
  int h = game.curWorld().terrain().height()*Terrain::quadSize;

  {
    GameObject& obj = game.curWorld().addObject("tower", 1);
    int p = 40-9;
    obj.setPosition( p*w/80, p*h/80 );
    }

  {
    GameObject& obj = game.curWorld().addObject("tower", 2);

    int p = 40+8;
    obj.setPosition( p*w/80, p*h/80 );
    }

  {
    GameObject& obj = game.curWorld().addObject("castle", 1);
    int p = 10;
    obj.setPosition( p*w/80, p*h/80 );
    }

  {
    GameObject& obj = game.curWorld().addObject("castle", 2);
    int p = 80-10;
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
  cen->layout().add( new UpgradePanel(res, game, plC[0]) );

  Tempest::Widget * box = new Tempest::Widget();
  box->layout().add( editPanel );
  box->layout().add( settingsPanel );
  cen->layout().add( box );
  cen->layout().add( new SpellPanel(res, game, plC[0]) );

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

  mmap = new Minimap(res,game, plC[0]);
  mmap->base->renderScene.bind( mainWidget->renderScene );
  mainWidget->updateView.bind( *mmap->base, &UnitView::updateView );
  mmap->base->onClick.bind(*mmap, &Minimap::hideInfo);

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

int DesertStrikeScenario::PlInfo::getParam(const std::string &p) const {
  if( p=="atack" )
    return atkGrade;

  if( p=="armor" )
    return armorGrade;

  return 0;
  }
