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
#include "algo/algo.h"

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
             PlInfo & pl,
             int tier ):NumButton(r), p(obj), pl(pl), tier(tier){
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
    //castleGrade;
    NumButton::paintEvent(e);
    }

  Tempest::signal<const ProtoObject&> onClick;

  const ProtoObject& p;
  PlInfo & pl;
  int tier;
  };

struct DesertStrikeScenario::GradeButton: public Button {
  GradeButton( Resource & r,
               PlInfo & p,
               const std::string& obj,
               const int t ):Button(r), type(t), pl(p){
    icon.data = res.pixmap(obj);
    texture.data = r.pixmap("gui/colors");
    //setText( obj.name );

    setMinimumSize( 50, 50 );
    setMaximumSize( 50, 50 );
    font = Font(15);

    clicked.bind(*this, &GradeButton::emitClick);
    }

  void emitClick(){
    onClick(type);
    }

  void paintEvent(Tempest::PaintEvent &e){
    //num = pl.getParam(type);
    Button::paintEvent(e);

    /*
    if( pl.maxBTime && type=="castle" ){
      int coolDown = h()*pl.btime/pl.maxBTime;

      Tempest::Painter p(e);
      p.setTexture( texture );
      p.setBlendMode( Tempest::alphaBlend );

      p.drawRect( 0, h()-coolDown, w(), coolDown,
                  2,        4, 1, 1 );
      }*/
    }

  Tempest::signal<int> onClick;
  int type;
  PlInfo & pl;
  Texture texture;
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
  struct GradeButton: public Button {
    GradeButton( Resource & r, PlInfo& pl ):Button(r), pl(pl) {
      setMinimumSize( 50, 50 );
      setMaximumSize( 50, 50 );

      Texture t;
      t.data       = res.pixmap("gui/icon/gold");
      texture.data = res.pixmap("gui/colors");
      //setText( obj.name );

      type = "castle";
      icon = t;
      }

    void paintEvent(Tempest::PaintEvent &e){
      //num = pl.getParam(type);
      Button::paintEvent(e);

      if( pl.maxBTime &&
          pl.queue.size() &&
          pl.queue[0]==type ){
        int coolDown = h()*pl.btime/pl.maxBTime;

        Tempest::Painter p(e);
        p.setTexture( texture );
        p.setBlendMode( Tempest::alphaBlend );

        p.drawRect( 0, h()-coolDown, w(), coolDown,
                    2,        4, 1, 1 );
        }
      }

    PlInfo& pl;
    std::string type;
    Texture texture;
    };

  struct Inf{
    Widget * widget;
    RichText * ledit, *info[2];

    GradeButton * grade;
    };

  Minimap( Resource &res,
           Game & game,
           PlInfo & pl ):MiniMapView(res), game(game), pl(pl){
    infID = 0;

    base = new UnitView(res);
    base->setLayout( Tempest::Vertical );

    buildBase( res, inf[0] );
    buildCas ( res, inf[1] );

    base->setVisible(0);
    }

  void buildBase( Resource &res, Inf & inf ){
    using namespace Tempest;

    Widget *panel = new Widget();
    panel->setLayout( Vertical );
    panel->setVisible(0);
    inf.widget = panel;
    inf.grade  = 0;

    Widget* w = new Widget();
    w->setLayout( Horizontal );

    BuyButton *btn = new BuyButton(res);
    btn->clicked.bind(*this, &Minimap::sell );
    w->layout().add( btn );

    DesertStrikeScenario::TranscurentPanel *t = new TranscurentPanel(res);
    t->setLayout( Vertical );
    t->layout().setMargin(6);
    inf.ledit = new RichText(res);
    t->layout().add( inf.ledit );
    w->layout().add( t );

    btn = new BuyButton(res);
    btn->clicked.bind(*this, &Minimap::buy );
    btn->icon.data = res.pixmap("gui/icon/atack");
    w->layout().add( btn );

    w->setMaximumSize( w->sizePolicy().maxSize.w, 50 );
    w->setSizePolicy( Preferred, FixedMax );

    panel->layout().add(w);
    w = new Widget();
    //w->setSizePolicy( Expanding );
    panel->layout().add( w );

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
      panel->layout().add( w );

      inf.info[i] = cost;
      }

    setLayout( Vertical );
    base->layout().add( panel );
    layout().add(base);

    //ledit->setEditable(0);

    //setFocusPolicy();
    }

  void buildCas( Resource &res, Inf & inf ){
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
    btn->clicked.bind(*this, &Minimap::grade );
    btn->icon.data = res.pixmap("gui/icon/build");
    w->layout().add( btn );
    inf.grade = btn;

    w->setMaximumSize( w->sizePolicy().maxSize.w, 50 );
    w->setSizePolicy( Preferred, FixedMax );

    panel->layout().add(w);
    w = new Widget();
    //w->setSizePolicy( Expanding );
    panel->layout().add( w );

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
      panel->layout().add( w );

      inf.info[i] = cost;
      }

    setLayout( Vertical );
    base->layout().add( panel );
    layout().add(base);

    //ledit->setEditable(0);

    //setFocusPolicy();
    }

  void paintEvent(Tempest::PaintEvent &e){
    MiniMapView::paintEvent(e);
    paintNested(e);
    }

  void mouseDownEvent(Tempest::MouseEvent &){
    hideInfo();
    }

  void setupUnit( const std::string & unit ){
    inf[infID].widget->setVisible(0);
    infID = 0;
    unitToBuy = unit;

    if( unit=="castle" ||
        unit=="house" ||
        unit=="tower" )
      infID = 1;

    base->setVisible(1);
    inf[infID].widget->setVisible(1);
    base->setFocus(1);
    base->setupUnit(game, unit);

    if( inf[infID].grade )
      inf[infID].grade->type = unit;

    updateValues();
    }

  void updateValues(){
    Inf &inf = this->inf[infID];

    if( unitToBuy.size()==0 )
      return;

    { std::wstringstream s;
      std::string name = "$(" +unitToBuy+")";
      s << Lang::tr(name) << std::endl << L" - " << game.prototype(unitToBuy).data.gold;
      inf.ledit->setText( s.str() );
      }

    { std::wstringstream s;
      if( game.prototype(unitToBuy).data.atk.size() ){
        s << game.prototype(unitToBuy).data.atk[0].damage <<" / "
          << game.prototype(unitToBuy).data.atk[0].range;
        }
      inf.info[0]->setText( s.str() );
      }

    { std::wstringstream s;
      s << game.prototype(unitToBuy).data.armor;
      inf.info[1]->setText( s.str() );
      }
    }

  void hideInfo(){
    base->setVisible( 0 );
    inf[infID].widget->setVisible(0);
    }

  void buy(){
    std::vector<char> data;
    ByteArraySerialize s(data, ByteArraySerialize::Write);

    s.write(game.player().number()-1);
    s.write( unitToBuy );
    s.write( 'b' );
    game.message( data );
    }

  void grade(){
    std::vector<char> data;
    ByteArraySerialize s(data, ByteArraySerialize::Write);

    s.write(game.player().number()-1);
    s.write( unitToBuy );
    s.write( 'g' );
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

  Inf inf[2];
  int infID;

  std::string unitToBuy;

  Game &game;
  PlInfo & pl;
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
    u = new DesertStrikeScenario::GradeButton(res, pl, "gui/icon/fire_strike", 0 );
    layout().add( u );

    u = new DesertStrikeScenario::GradeButton(res, pl, "gui/icon/blink", 1 );
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
    //setMaximumSize(250, 200);

    setLayout( Vertical );
    layout().setMargin(15);

    std::fill(layers, layers+4, (Widget*)0);

    const char * pr[3][4] = {
      {"gelion", "pikeman", "incvisitor", "water_mage"},
      {"fire_mage", "balista"},
      {"fire_element", "golem"}
      };

    const char * cas[3][4] = {
      {"castle"},
      {"house"},
      {"tower"}
      };

    layers[0] = mkPanel<3,4>(pl, cas);
    layers[1] = mkPanel<3,4>(pl, pr );
    layout().add( layers[0] );
    layout().add( layers[1] );

    setTab(0);
    }

  void onUnit( const ProtoObject & p ){
    setupBuyPanel(p.name);
    }

  void setupBuyPanel( const std::string & s ){
    mmap->setupUnit(s);
    }

  template< int w, int h >
  Widget* mkPanel( PlInfo & pl, const char * pr[w][h] ){
    using namespace Tempest;

    Widget *w = new Widget();
    w->setLayout(Vertical);

    for( int i=0; i<3; ++i ){
      Widget* l = new Widget();
      l->setLayout( Horizontal );

      for( int r=0; r<4; ++r ){
        if( pr[i][r] ){
          const ProtoObject & obj = game.prototype(pr[i][r]);

          DesertStrikeScenario::BuyButton * u =
              new DesertStrikeScenario::BuyButton(res, obj, pl, i);
          u->onClick.bind( *this, &BuyUnitPanel::onUnit );
          l->layout().add( u );
          }
        }
      w->layout().add(l);
      }

    return w;
    }

  void setTab( int id ){
    for( int i=0; i<4; ++i )
      if( layers[i] )
        layers[i]->setVisible(i==id);
    }

  Game &game;
  Minimap * mmap;

  Widget* layers[4];
  };

struct DesertStrikeScenario::UpgradePanel: public TranscurentPanel {
  UpgradePanel( Resource & res,
                Game & game,
                PlInfo & pl,
                DesertStrikeScenario::BuyUnitPanel *mmap )
    :TranscurentPanel(res), game(game), mmap(mmap){
    using namespace Tempest;

    setMinimumSize(75, 200);
    setMaximumSize(75, 200);
    layout().setMargin(15);

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
      }

    }

  void buy( const int grade ){
    /*
    std::vector<char> data;
    ByteArraySerialize s(data, ByteArraySerialize::Write);

    s.write(game.player().number()-1);
    s.write( grade );
    s.write( 'g' );
    game.message( data );*/

    mmap->setTab(grade);
    }

  Game & game;
  DesertStrikeScenario::BuyUnitPanel *mmap;
  };

DesertStrikeScenario::DesertStrikeScenario(Game &g, MainGui &ui, BehaviorMSGQueue &msg)
  :Scenario(g,ui, msg){
  tNum     = 0;
  interval = 1000;
  isMouseTracking = false;

  plC[0].units["pikeman"] = 3;
  plC[1].units["pikeman"] = 3;

  game.player(1).setGold(350);
  game.player(2).setGold(350);

  gui.update();
  g.prototypes().load("campagin/td.xml");
  }

DesertStrikeScenario::~DesertStrikeScenario() {
  game.prototypes().unload();
  }

void DesertStrikeScenario::mouseDownEvent( Tempest::MouseEvent &e ) {
  mpos3d          = unProjectRz( e.x, e.y, moveZ );
  isMouseTracking = true;
  }

void DesertStrikeScenario::mouseUpEvent( Tempest::MouseEvent & e ) {
  isMouseTracking = false;

  World  &world  = game.curWorld();
  Player &player = game.player();

  F3 v = unProject( e.x, e.y );

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

void DesertStrikeScenario::mouseMoveEvent( Tempest::MouseEvent &e ) {
  if( !isMouseTracking && gui.mouseMoveEvent(e) ){
    acceptMouseObj = false;
    return;
    }

  if( isMouseTracking ){
    F3 m = unProject( e.x, e.y, moveZ );
    game.curWorld().moveCamera( mpos3d.data[0]-m.data[0],
                                mpos3d.data[1]-m.data[1]);
    mpos3d = unProject( e.x, e.y, moveZ );
    }

  updateMousePos(e);
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
      //++plC[ 1 ].units[name];
      }
    } else
  if( ch=='s' && plC[ pl ].units[name]>0 ){
    --plC[ pl ].units[name];

    //debug
    --plC[ 1 ].units[name];
    } else
  if( ch=='g' && plC[pl].queue.size()==0 ){
    plC[pl].queue.push_back(name);
    plC[pl].maxBTime = 500;
    plC[pl].btime    = plC[pl].maxBTime;
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
             p2*w/80, p2*h/80,
             0 );
    }

  if( tNum%interval==0 ){
    mkUnits( 2,
             p2*w/80, p2*h/80,
             p1*w/80, p1*h/80,
             1 );
    }

  ++tNum;
  if( tNum/interval>=2 ){
    tNum = 0;
    aiTick(1);
    }

  for( int i=0; i<2; ++i ){
    if( plC[i].btime>0 ){
      mmap->update();
      --plC[i].btime;
      } else
    if( plC[i].btime==0 ){
      mmap->update();

      if( plC[i].queue.size() ){
        plC[i].btime = 500;

        if( plC[i].queue.front()=="house" ){
          ++plC[i].economyGrade;
          }

        for( size_t r=0; r+1<plC[i].queue.size(); ++r ){
          plC[i].queue[r] = plC[i].queue[r+1];
          }
        plC[i].queue.pop_back();
        }
      }

    if( tNum%10==0 ){
      if( !(plC[i].btime && plC[i].queue.size() && plC[i].queue[0]=="house") )
        game.player(i+1).addGold( 1+plC[i].economyGrade );
      }
    }
  }

void DesertStrikeScenario::mkUnits( int pl,
                                    int   x, int   y,
                                    int tgX, int tgY,
                                    bool rev ){
  --pl;

  std::map<std::string, int>::iterator u, e = plC[pl].units.end();
  int count = 0;

  for( u = plC[pl].units.begin(); u!=e; ++u )
    count += u->second;

  int qc = 1;
  //std::max(Math::sqrt(count)-1,1);
  while( qc*qc<count )
    ++qc;

  int id = 0;
  for( u = plC[pl].units.begin(); u!=e; ++u ){
    int& c = plC[pl].realCount[u->first];
    for( int i=0; i<u->second; ++i ){
      if( c < 2*u->second ){
        GameObject& obj = game.curWorld().addObject(u->first, pl+1);
        ++c;
        ++id;

        if( rev ){
          obj.setPosition( x+(qc/2 - id%qc)*Terrain::quadSize,
                           y+(qc/2 - id/qc)*Terrain::quadSize );
          } else {
          obj.setPosition( x+(id%qc - qc/2)*Terrain::quadSize,
                           y+(id/qc - qc/2)*Terrain::quadSize );
          }

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

  p.maxSize.h = gold->sizePolicy().maxSize.h;
  top->setSizePolicy(p);

  editPanel     = createEditPanel(mainWidget, res);
  settingsPanel = createSettingsPanel(mainWidget, res);

  cen->setLayout( Tempest::Horizontal );
  cen->layout().add( new MissionTargets(game, res) );
  cen->layout().add( new UpgradePanel(res, game, plC[0], buyUnitPanel) );

  Tempest::Widget * box = new Tempest::Widget();
  box->layout().add( editPanel );
  box->layout().add( settingsPanel );
  cen->layout().add( box );
  cen->layout().add( new SpellPanel(res, game, plC[0]) );

  cen->useScissor( false );
  box->useScissor( false );

  editPanel->setVisible(0);
  //settingsPanel->setVisible(0);

  showEditPanel.activated.bind( *this, &DesertStrikeScenario::toogleEditPanel );
  showSettings. activated.bind( *this, &DesertStrikeScenario::toogleSettingsPanel );
  }

Tempest::Widget *DesertStrikeScenario::createConsole( InGameControls *mainWidget,
                                                      BehaviorMSGQueue &  ) {
  using namespace Tempest;

  Tempest::Widget* console = new Widget();
  console->setMaximumSize( SizePolicy::maxWidgetSize().w, 220 );
  console->setMinimumSize( 0, 200);

  SizePolicy p;
  p.minSize = Tempest::Size(200, 200);
  p.maxSize = Tempest::Size(200, 200);

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

  buyUnitPanel = new BuyUnitPanel( res, game, plC[0], mmap );
  console->layout().add( buyUnitPanel );
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

void DesertStrikeScenario::aiTick( int pl ) {
  struct BuildElement{
    std::string src;
    int scount;
    std::string dst;
    int dcount;
    };

  BuildElement b[] = {
    {"pikeman",    3, "incvisitor", 1 },
    {"pikeman",    5, "gelion",      1 },
    {"incvisitor", 1, "pikeman",    3 },

    {"incvisitor", 3, "fire_mage",  3 },
    {"", 0, "", 0}
  };

  for( int i=0; i<2; ++i )
    if( pl!=i ){
      for( int r=0; ; ++r ){
        const BuildElement& e = b[r];
        if( e.src.size()==0 )
          return;

        int c = plC[i].units[e.src];
        c*=e.dcount;
        c/=e.scount;

        for( int q=plC[ pl ].units[e.dst]; q<c; ++q ){
          int gold = game.prototype(e.dst).data.gold;
          if( game.player(pl+1).gold() >= gold ){
            ++plC[ pl ].units[e.dst];
            game.player(pl+1).setGold( game.player(pl+1).gold() - gold );
            }
          }
        }
      }
  }
