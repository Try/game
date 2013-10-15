#include "desertstrikescenariowidgets.h"

#include <Tempest/Application>

#include "gui/unitinfo.h"
#include "gui/minimapview.h"

#include <sstream>
#include "gui/lineedit.h"
#include "gui/unitview.h"
#include "gui/richtext.h"
#include "gui/listbox.h"

#include "lang/lang.h"

#include "util/math.h"
#include "algo/algo.h"

#include "util/bytearrayserialize.h"

#include "util/weakworldptr.h"

DesertStrikeScenario::NumButton::NumButton( Resource & r ):Button(r) {
  setMinimumSize( 55, 55 );
  setMaximumSize( 55, 55 );

  numFrame = res.pixmap("gui/hintFrame");
  num = 0;
  }

void DesertStrikeScenario::NumButton::paintEvent(Tempest::PaintEvent &e){
  using namespace Tempest;

  Button::paintEvent(e);

  Painter p(e);
  p.setFont(font);
  p.setBlendMode(alphaBlend);

  std::stringstream s;
  s << num;

  p.setTexture(numFrame);
  Size tsz = font.textSize(s.str());
  int nw = numFrame.w(),
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

DesertStrikeScenario::BuyButton::BuyButton(Resource & r,
           const ProtoObject& obj,
           DPlayer &pl,
           int tier ):NumButton(r), p(obj), pl(pl), tier(tier){
  icon        = res.pixmap("gui/icon/"+obj.name);
  texture     = res.pixmap("gui/colors");
  //setText( obj.name );

  font = Tempest::Font(15);

  clicked.bind( this, &BuyButton::emitClick);
  }

void DesertStrikeScenario::BuyButton::emitClick(){
  onClick(p);
  }

void DesertStrikeScenario::BuyButton::paintEvent(Tempest::PaintEvent &e){
  num = pl.getParam(p.name);//.units[this->p.name];
  //castleGrade;
  NumButton::paintEvent(e);

  if( tier > pl.castleGrade ){
    Tempest::Painter p(e);
    p.setTexture( texture );
    p.setBlendMode( Tempest::alphaBlend );

    p.drawRect( 0, 0, w(), h(),
                0, 4, 1, 1 );
    }
  }

DesertStrikeScenario::GradeButton::GradeButton( Resource & r,
             DPlayer & p,
             const std::string& obj,
             const int t ):Button(r), type(t), pl(p){
  icon    = res.pixmap(obj);
  texture = res.pixmap("gui/colors");
  //setText( obj.name );

  setMinimumSize( 55, 55 );
  setMaximumSize( 55, 55 );
  setFont( Tempest::Font(15) );

  clicked.bind( this, &GradeButton::emitClick);
  }

void DesertStrikeScenario::GradeButton::emitClick(){
  onClick(type);
  }

void DesertStrikeScenario::GradeButton::paintEvent(Tempest::PaintEvent &e){
  //num = pl.getParam(type);
  Button::paintEvent(e);
  }

DesertStrikeScenario::TranscurentPanel::TranscurentPanel( Resource & res ):res(res){
  frame = res.pixmap("gui/hintFrame");
  }

void DesertStrikeScenario::TranscurentPanel::paintEvent(Tempest::PaintEvent &e){
  Tempest::Painter p(e);

  MainGui::drawFrame(p, frame, Tempest::Point(), size() );

  paintNested(e);
  }

void DesertStrikeScenario::TranscurentPanel::mouseDownEvent(Tempest::MouseEvent &e) {
  e.accept();
  }

struct DesertStrikeScenario::Minimap::BuyButton: public Button {
  BuyButton( Resource & r ):Button(r) {
    setMinimumSize( 55, 55 );
    setMaximumSize( 55, 55 );

    Tempest::Sprite t;
    t = res.pixmap("gui/icon/gold");
    //setText( obj.name );

    icon = t;
    }
  };

struct DesertStrikeScenario::Minimap::GradeButton: public Button {
  GradeButton( Resource & r, DPlayer& pl ):Button(r), pl(pl) {
    setMinimumSize( 55, 55 );
    setMaximumSize( 55, 55 );

    Tempest::Sprite t;
    t       = res.pixmap("gui/icon/gold");
    texture = res.pixmap("gui/colors");
    //setText( obj.name );

    type = "castle";
    icon = t;
    }

  void paintEvent(Tempest::PaintEvent &e){
    //num = pl.getParam(type);
    Button::paintEvent(e);

    for( size_t i=0; i<pl.queue.size(); ++i ){
      if( pl.queue[i].name==type ){
        int coolDown = h()*pl.queue[i].btime/pl.queue[i].maxBTime;

        Tempest::Painter p(e);
        p.setTexture( texture );
        p.setBlendMode( Tempest::alphaBlend );

        p.drawRect( 0, h()-coolDown, w(), coolDown,
                    2,        4, 1, 1 );
        }
      }
    }

  DPlayer& pl;
  std::string type;
  Tempest::Sprite texture;
  };

DesertStrikeScenario::Minimap::Minimap( Resource &res,
         Game & game,
         DPlayer & pl ):MiniMapView(res), game(game), pl(pl){
  infID = 0;

  base = new UnitView(res);
  base->setLayout( Tempest::Vertical );

  buildBase  ( res, inf[0] );
  buildCas   ( res, inf[1] );
  buildGrade ( res, inf[2] );

  base->setVisible(0);
  }

void DesertStrikeScenario::Minimap::buildBase( Resource &res, Inf & inf ){
  using namespace Tempest;

  Widget *panel = new Widget();
  panel->setLayout( Vertical );
  panel->setVisible(0);
  inf.widget = panel;
  inf.grade  = 0;

  Widget* w = new Widget();
  w->setLayout( Horizontal );

  BuyButton *btn = new BuyButton(res);
  btn->clicked.bind( this, &Minimap::sell );
  btn->icon = res.pixmap("gui/minus");
  w->layout().add( btn );

  Widget *t = new Widget();
  t->setLayout( Vertical );
  t->layout().setMargin(6);
  inf.ledit = new RichText(res);
  t->layout().add( inf.ledit );
  w->layout().add( t );

  btn = new BuyButton(res);
  btn->clicked.bind( this, &Minimap::buy );
  btn->icon = res.pixmap("gui/plus");
  w->layout().add( btn );

  w->setMaximumSize( w->sizePolicy().maxSize.w, 50 );
  w->setSizePolicy( Preferred, FixedMax );

  panel->layout().add(w);
  w = new Widget();
  //w->setSizePolicy( Expanding );
  panel->layout().add( w );  
  mkInfoPanel(res, inf, panel);

  setLayout( Vertical );
  base->layout().add( panel );
  layout().add(base);

  //ledit->setEditable(0);

  //setFocusPolicy();
  }

void DesertStrikeScenario::Minimap::buildCas( Resource &res, Inf & inf ){
  using namespace Tempest;

  Widget *panel = new Widget();
  panel->setLayout( Vertical );
  panel->setVisible(0);
  inf.widget = panel;

  Widget* w = new Widget();
  w->setLayout( Horizontal );

  DesertStrikeScenario::TranscurentPanel *t = new TranscurentPanel(res);
  t->setLayout( Vertical );
  t->layout().setMargin(6);
  inf.ledit = new RichText(res);
  t->layout().add( inf.ledit );
  w->layout().add( t );

  GradeButton *btn = 0;
  btn = new GradeButton(res, pl);
  btn->clicked.bind( this, &Minimap::grade );
  btn->icon = res.pixmap("gui/icon/build");
  w->layout().add( btn );
  inf.grade = btn;

  w->setMaximumSize( w->sizePolicy().maxSize.w, 50 );
  w->setSizePolicy( Preferred, FixedMax );

  panel->layout().add(w);
  w = new Widget();
  //w->setSizePolicy( Expanding );
  panel->layout().add( w );
  mkInfoPanel(res, inf, panel);

  setLayout( Vertical );
  base->layout().add( panel );
  layout().add(base);

  //ledit->setEditable(0);

  //setFocusPolicy();
  }

void DesertStrikeScenario::Minimap::buildGrade( Resource &res, Inf & inf ){
  using namespace Tempest;

  Widget *panel = new Widget();
  panel->setLayout( Vertical );
  panel->setVisible(0);
  inf.widget = panel;

  Widget* w = new Widget();
  w->setLayout( Horizontal );

  DesertStrikeScenario::TranscurentPanel *t = new TranscurentPanel(res);
  t->setLayout( Vertical );
  t->layout().setMargin(6);
  inf.ledit = new RichText(res);
  t->layout().add( inf.ledit );
  w->layout().add( t );

  GradeButton *btn = 0;
  btn = new GradeButton(res, pl);
  btn->clicked.bind( this, &Minimap::grade );
  btn->icon = res.pixmap("gui/icon/actions/plus");
  w->layout().add( btn );
  inf.grade = btn;

  w->setMaximumSize( w->sizePolicy().maxSize.w, 50 );
  w->setSizePolicy( Preferred, FixedMax );

  panel->layout().add(w);
  w = new Widget();
  //w->setSizePolicy( Expanding );
  panel->layout().add( w );
  mkInfoPanel(res, inf, panel);

  setLayout( Vertical );
  base->layout().add( panel );
  layout().add(base);

  //ledit->setEditable(0);

  //setFocusPolicy();
  }

void DesertStrikeScenario::Minimap::mkInfoPanel( Resource &res,
                                                 Inf &inf,
                                                 Widget* panel ){
  using namespace Tempest;

  const char* icon[][2] = {
    {"gui/icon/atack", "gui/item/shield"},
    {"gui/heart",      "gui/coin"}
    };

  Widget *w = 0;
  for( int i=0; i<2; ++i ){
    w = new Widget();
    w->setMaximumSize( w->sizePolicy().maxSize.w, 25 );
    w->setSizePolicy( Preferred, FixedMax );

    w->setLayout( Horizontal );

    for( int r=0; r<2; ++r ){
      RichText * cost = new RichText(res);
      cost->setText(L"<s>123</s>");

      BuyButton * btn = new BuyButton(res);
      btn->setMaximumSize(25);
      btn->icon = res.pixmap(icon[i][r]);

      w->layout().add( btn );
      w->layout().add( cost );

      inf.info[i][r] = cost;
      }

    panel->layout().add( w );
    }
  }

void DesertStrikeScenario::Minimap::paintEvent(Tempest::PaintEvent &e){
  MiniMapView::paintEvent(e);
  paintNested(e);
  }

void DesertStrikeScenario::Minimap::mouseDownEvent(Tempest::MouseEvent &){
  hideInfo();
  }

void DesertStrikeScenario::Minimap::setupUnit( const std::string & unit ){
  inf[infID].widget->setVisible(0);
  infID = 0;
  unitToBuy = unit;

  if( unit=="castle" ||
      unit=="house" ||
      unit=="tower" ){
    infID = 1;
    onBuilding(unit);
    }

  base->setVisible(1);
  inf[infID].widget->setVisible(1);
  base->setFocus(1);
  base->setupUnit(game, unit);

  if( inf[infID].grade )
    inf[infID].grade->type = unit;

  onUnit( unit );
  updateValues();
  }

void DesertStrikeScenario::Minimap::setGrade() {
  inf[infID].widget->setVisible(0);
  infID = 2;
  unitToBuy = "";

  base->setVisible(1);
  inf[infID].widget->setVisible(1);
  base->setFocus(1);
  base->setupUnit(game, "");

  if( inf[infID].grade )
    inf[infID].grade->type = "";

  inf[infID].ledit->setText( Lang::tr("melee_atack") );
  //updateValues();
  }

void DesertStrikeScenario::Minimap::updateValues(){
  Inf &inf = this->inf[infID];

  if( unitToBuy.size()==0 )
    return;

  { std::string name = "$(unit/" +unitToBuy+")";
    inf.ledit->setText( Lang::tr(name) );
    }

  { std::wstringstream s;
    if( game.prototype(unitToBuy).data.atk.size() ){
      s << game.prototype(unitToBuy).data.atk[0].damage <<" / "
        << game.prototype(unitToBuy).data.atk[0].range;
      }
    inf.info[0][0]->setText( s.str() );
    }

  { std::wstringstream s;
    s << game.prototype(unitToBuy).data.armor;
    inf.info[0][1]->setText( s.str() );
    }

  { std::wstringstream s;
    s << game.prototype(unitToBuy).data.maxHp;
    inf.info[1][0]->setText( s.str() );
    }

  { std::wstringstream s;
    s << game.prototype(unitToBuy).data.gold;
    inf.info[1][1]->setText( s.str() );
    }
  }

void DesertStrikeScenario::Minimap::hideInfo(){
  base->setVisible( 0 );
  inf[infID].widget->setVisible(0);
  }

void DesertStrikeScenario::Minimap::buy(){
  std::vector<char> data;
  ByteArraySerialize s(data, ByteArraySerialize::Write);

  s.write( game.player().number()-1 );
  s.write( unitToBuy );
  s.write( 'b' );
  game.message( data );
  }

void DesertStrikeScenario::Minimap::grade(){
  std::vector<char> data;
  ByteArraySerialize s(data, ByteArraySerialize::Write);

  s.write(game.player().number()-1);
  s.write( unitToBuy );
  s.write( 'g' );
  game.message( data );
  }

void DesertStrikeScenario::Minimap::sell(){
  std::vector<char> data;
  ByteArraySerialize s(data, ByteArraySerialize::Write);

  s.write(game.player().number()-1);
  s.write( unitToBuy );
  s.write( 's' );
  game.message( data );
  }

struct DesertStrikeScenario::SpellPanel::CameraButton: public Button {
  CameraButton( Resource & r,
                Game& game ):Button(r), game(game) {
    selection = res.pixmap("gui/buttonHightlight");
    }

  Tempest::Sprite selection;
  Game& game;

  void paintEvent(Tempest::PaintEvent &e){
    using namespace Tempest;
    Button::paintEvent(e);

    Tempest::Painter p(e);

    DesertStrikeScenario& s = (DesertStrikeScenario&)game.scenario();
    if( s.hasVTracking ){
      p.setTexture(selection);

      p.setBlendMode( Tempest::addBlend );

      p.drawRect( Rect(4,4,w()-8,h()-8),
                  Rect(0,0, selection.w(), selection.h()) );
      }
    }

  };

struct DesertStrikeScenario::SpellPanel::SpellButton: public GradeButton {
  SpellButton( Resource & r,
               DPlayer & pl,
               Game &g,
               const std::string& obj,
               const std::string& taget,
               const int t ):GradeButton(r,pl,obj,t), taget(taget), game(g) {
    tagetID   = g.prototypes().spell( taget ).id;
    selection = res.pixmap("gui/buttonHightlight");

    GradeButton::clicked.bind(this, &SpellButton::emitClick);
    timer.timeout.bind(this, &SpellButton::updateValues);
    timer.start(100);
    }

  void emitClick(){
    clicked(taget);
    }

  Tempest::Timer timer;

  int coolDown;
  size_t tagetID;
  std::string taget;
  Game &game;
  Tempest::Sprite selection;

  Tempest::signal<std::string> clicked;

  void paintEvent(Tempest::PaintEvent &e){
    using namespace Tempest;
    GradeButton::paintEvent(e);

    Tempest::Painter p(e);

    DesertStrikeScenario& s = (DesertStrikeScenario&)game.scenario();
    if( s.spellToCast==taget ){
      p.setTexture(selection);

      p.setBlendMode( Tempest::addBlend );

      p.drawRect( Rect(4,4,w()-8,h()-8),
                  Rect(0,0, selection.w(), selection.h()) );
      }

    p.setTexture( texture );
    p.setBlendMode( Tempest::alphaBlend );
    p.drawRect( 0, h()-coolDown, w(), coolDown,
                2,        4, 1, 1 );
    }

  void updateValues(){
    //assert(u0);
    int maxT = game.prototypes().spell(taget).coolDown;

    int mcoolDown = maxT;

    //auto s = player().selected();
    bool v = false;
    for( size_t i=0; i<game.player().unitsCount(); ++i ){
      GameObject & obj = game.player().unit(i);
      int t = obj.coolDown( tagetID );
      if( t>=0 ){
        mcoolDown = std::min(t,mcoolDown);
        v = true;
        }
      }

    setVisible(v);

    mcoolDown = mcoolDown*h()/maxT;
    if( mcoolDown!=coolDown ){
      coolDown = mcoolDown;
      update();
      }
    }

  };

DesertStrikeScenario::SpellPanel::SpellPanel( Resource & res,
                                              Game & game,
                                              DPlayer & pl )
  :TranscurentPanel(res), game(game){
  using namespace Tempest;

  setMinimumSize(75, 200);
  setMaximumSize(75, 200);
  layout().setMargin(15);

  setSizePolicy( FixedMin );
  setLayout( Vertical );

  Button * c = new CameraButton(res, game);
  c->setMinimumSize( 55, 55 );
  c->setMaximumSize( 55, 55 );
  c->icon = res.pixmap("gui/icon/camera");
  c->clicked.bind( toogleCameraMode );
  layout().add( c );

  const char* spell[] = {
    "fire_strike",
    "fire_storm",
    0
    };

  for( int i=0; spell[i]; ++i ){
    SpellButton * u = 0;
    std::string icon = "gui/icon/";
    icon += spell[i];

    u = new SpellButton(res, pl, game, icon, spell[i], 0 );
    u->onClick.bind( this, &SpellPanel::spell );
    Spell::Mode m = game.prototypes().spell( spell[i] ).mode;

    if( m==Spell::CastToCoord )
      u->clicked.bind( this, &SpellPanel::setupHook );

    if( m==Spell::CastToUnit )
      u->clicked.bind( this, &SpellPanel::setupHookU );
    layout().add( u );
    }
  }

void DesertStrikeScenario::SpellPanel::setupHook(const std::string &spell) {
  std::vector<char> data;
  ByteArraySerialize s(data, ByteArraySerialize::Write);

  s.write( game.player().number()-1 );
  s.write( spell );
  s.write( 'm' );
  //s.write( int(CastToCoord) );
  game.message( data );
  }

void DesertStrikeScenario::SpellPanel::setupHookU(const std::string &spell) {
  std::vector<char> data;
  ByteArraySerialize s(data, ByteArraySerialize::Write);

  s.write( game.player().number()-1 );
  s.write( spell );
  s.write( 'm' );
  //s.write( int(CastToUnit) );
  game.message( data );
  }

void DesertStrikeScenario::SpellPanel::spell( int i ){
  std::vector<char> data;
  ByteArraySerialize s(data, ByteArraySerialize::Write);

  s.write( game.player().number()-1 );
  s.write( i );
  s.write( 'm' );
  game.message( data );
  }

DesertStrikeScenario::BuyUnitPanel::BuyUnitPanel( Resource & res,
              Game & game,
              DPlayer & pl,
              Minimap * mmap): TranscurentPanel(res), game(game), mmap(mmap) {
  using namespace Tempest;
  setMinimumSize(250, 200);
  //setMaximumSize(250, 200);

  setLayout( Vertical );
  layout().setMargin(10);

  for( int i=0; i<4; ++i )
    layers[i].w = 0;

  auto pr = DesertStrikeScenario::units;

  const char * cas[3][4] = {
    {"castle", "house" },
    { },
    { }
    };

  const char * gr[3][4] = {
    {"atack.melee", "atack.range", "atack.magic" },
    { },
    {}
    };

  layers[0] = mkPanel<3,4>(pl, cas, &BuyUnitPanel::mkBuyCasBtn   );
  layers[1] = mkPanel<3,4>(pl, pr , &BuyUnitPanel::mkBuyUnitBtn  );
  layers[2] = mkPanel<3,4>(pl, gr , &BuyUnitPanel::mkBuyGradeBtn );

  layout().add( layers[0].w );
  layout().add( layers[1].w );
  layout().add( layers[2].w );

  setTab(1);
  }

Button *DesertStrikeScenario::BuyUnitPanel::mkBuyUnitBtn( Resource &res,
                                                          const char *sobj,
                                                          DPlayer &pl,
                                                          int tier ) {
  const ProtoObject & obj = game.prototype(sobj);

  BuyButton * u = new BuyButton(res, obj, pl, tier);
  u->onClick.bind( *this, &BuyUnitPanel::onUnit );

  return u;
  }

Button *DesertStrikeScenario::BuyUnitPanel::mkBuyCasBtn( Resource &res,
                                                         const char *sobj,
                                                         DPlayer &pl,
                                                         int /*tier*/ ) {
  const ProtoObject & obj = game.prototype(sobj);

  BuyButton * u = new BuyButton(res, obj, pl, 0);
  u->onClick.bind( *this, &BuyUnitPanel::onUnit );

  return u;
  }

Button *DesertStrikeScenario::BuyUnitPanel::mkBuyGradeBtn( Resource &res,
                                                           const char *sobj,
                                                           DPlayer &,
                                                           int /*tier*/ ) {
  //const ProtoObject & obj = game.prototype(sobj);

  NumButton * u = new NumButton(res);
  u->icon = res.pixmap( std::string("gui/icon/") + sobj);

  u->clicked.bind( *this, &BuyUnitPanel::onGrade );

  return u;
  }

void DesertStrikeScenario::BuyUnitPanel::onUnit( const ProtoObject & p ){
  setupBuyPanel(p.name);
  }

void DesertStrikeScenario::BuyUnitPanel::onGrade() {
  mmap->setGrade();
  }

void DesertStrikeScenario::BuyUnitPanel::setupBuyPanel( const std::string & s ){
  mmap->setupUnit(s);
  }

void DesertStrikeScenario::BuyUnitPanel::setup(int id, bool e[3][4]) {
  for( int i=0; i<3; ++i )
    for( int r=0; r<4; ++r )
      if( layers[id].btns[i][r] )
        layers[id].btns[i][r]->setVisible(e[i][r]);
  }

void DesertStrikeScenario::BuyUnitPanel::setTab( int id ){
  for( int i=0; i<4; ++i )
    if( layers[i].w )
      layers[i].w->setVisible(i==id);
  }

DesertStrikeScenario::UpgradePanel::UpgradePanel( Resource & res,
              Game & game,
              DPlayer & pl,
              DesertStrikeScenario::BuyUnitPanel *mmap )
  :TranscurentPanel(res), game(game), mmap(mmap){
  using namespace Tempest;

  setMinimumSize(75, 200);
  setMaximumSize(75, 200);
  layout().setMargin(10);

  setSizePolicy( FixedMin );
  setLayout( Vertical );

  const char* gr[3][2] = {
    {"gui/icon/castle", "castle"},
    {"gui/icon/atack",  "atack"},
    {"gui/item/shield", "armor"}
    };

  DesertStrikeScenario::GradeButton * u = 0;
  for( int i=0; i<3; ++i ){
    u = new DesertStrikeScenario::GradeButton(res, pl, gr[i][0], i );
    layout().add( u );
    u->onClick.bind( *this, &UpgradePanel::buy );
    u->onClick.bind( onPage );
    }

  }

void DesertStrikeScenario::UpgradePanel::buy( const int grade ){
  mmap->setTab(grade);
  }

DesertStrikeScenario::CentralPanel::CentralPanel( DesertStrikeScenario &ds,
                                                  Resource &res ):res(res), ds(ds) {
  bg    = res.pixmap("gui/spark");
  cride = res.pixmap("gui/cride");

  time = 0;
  cl.set(0,0,0,1);
  }

void DesertStrikeScenario::CentralPanel::setRemTime(int t) {
  if( time!=t ){
    time = t;
    update();
    }
  }

void DesertStrikeScenario::CentralPanel::setColor(const Tempest::Color &c) {
  if( cl!=c ){
    cl = c;
    update();
    }
  }

void DesertStrikeScenario::CentralPanel::paintEvent(Tempest::PaintEvent &e) {
  paintNested(e);

  Tempest::Painter p(e);

  float k = 0.5;

  //PainterGUI& pt = (PainterGUI&)p.device();
  p.setColor( k*cl.r(), k*cl.g(), k*cl.b(), 1 );
  p.setBlendMode( Tempest::addBlend );
  p.setScissor( Tempest::Rect(0,0,w(),h()) );

  p.setTexture( bg );
  int h0 = 3*bg.h()/4;
  p.drawRect( w()/2-bg.w()/2, -h0/2,
              bg.w(), h0,
              0,0, bg.w(), bg.h());
  p.setColor(1,1,1,0.5);

  p.setBlendMode( Tempest::alphaBlend );
  p.setTexture(cride);
  float sz = 0.5;
  p.drawRect( w()/2-sz*cride.w()/2, -sz*cride.h()/2,
              sz*cride.w(), sz*cride.h(),
              0, 0, cride.w(), cride.h() );
  p.setColor(1,1,1,1);

  Tempest::Font font;
  p.setFont(font);

  std::stringstream ss;
  ss << time;
  const std::string s = ss.str();//ds.tNum/40);
  Tempest::Size r = font.textSize(s);
  p.drawText( w()/2-r.w/2, 0, s );
}


DesertStrikeScenario::WinLoseScreen::WinLoseScreen( Resource &res,
                                                    Tempest::Widget *owner,
                                                    Game & game )
  :ModalWindow(res, owner), isWin(0){
  using namespace Tempest;

  defaultMainMenu = true;

  Button *cont = new Button(res);
  cont->setText( Lang::tr("$(game_menu/to_mainmenu)") );
  cont->clicked.bind( game, &Game::showMainMenu );

  Widget* w = new Widget;
  w->setMaximumSize(500, 500);
  w->setLayout( Vertical );
  w->layout().add( new Widget() );
  w->layout().add( cont );

  setLayout( Vertical );
  layout().add( new Widget() );
  layout().add( w );
  layout().add( new Widget() );

  stime = Tempest::Application::tickCount();
  }

void DesertStrikeScenario::WinLoseScreen::paintEvent(Tempest::PaintEvent &e) {
  Tempest::Painter p(e);

  const uint64_t dt = 1000*3;

  double k = std::min(1.0, (Tempest::Application::tickCount()-stime)/double(dt));
  p.setColor(0,0,0, k );
  p.setBlendMode( Tempest::alphaBlend );
  p.drawRect(0,0,w(),h());

  p.setColor( 1,1,1,1 );
  p.setFont( Tempest::Font(50) );

  const std::wstring ws = isWin ? Lang::tr("$(desertstrike/win)")
                                : Lang::tr("$(desertstrike/lose)");
  p.drawText(0,0,w(),h(), ws, Tempest::AlignHCenter|Tempest::AlignVCenter );
  paintNested(e);

  if( k<1 )
    update();
  }


DesertStrikeScenario::Hint::Hint(Resource &res, Tempest::Widget *owner, Game &game)
  :ModalWindow(res,owner), game(game) {
  setLayout( Tempest::Vertical );

  struct CPanel:Panel{
    CPanel( Resource &res, Widget* ow ):Panel(res),owner(ow), closeRq(0){}

    void mouseDownEvent(Tempest::MouseEvent &e){
      closeRq = 1;
      ppress = e.pos();
      }

    void mouseDragEvent(Tempest::MouseEvent &e){
      if( (e.pos()-ppress).manhattanLength()>15 )
        closeRq = 0;
      }

    void mouseUpEvent(Tempest::MouseEvent &){
      if( closeRq )
        owner->deleteLater();
      }

    void paintFrame( Tempest::Painter & p){
      Panel::paintFrame(p);
      p.setColor(0,0,0,0.3);
      p.setBlendMode( Tempest::alphaBlend );

      int sz = 8;
      p.drawRect( sz, sz, w()-sz*2, h()-sz*2 );
      }

    Widget * owner;
    bool     closeRq;
    Tempest::Point ppress;
    };
  Panel *p = new CPanel(res, this);

  RichText * t = new RichText(res);
  t->setText( Lang::tr(L"$(desertstrike/tutorial/intro)") );

  p->layout().add(t);
  p->setMargin(10);
  p->setMaximumSize(480, 300);
  p->setMinimumSize(480, 300);

  layout().add( new Widget );
  layout().add( p );
  layout().add( new Widget );
  }

DesertStrikeScenario::Hint::~Hint() {
  game.unsetPause();
  }

void DesertStrikeScenario::Hint::paintEvent(Tempest::PaintEvent &e) {
  ModalWindow::paintEvent(e);
  }
