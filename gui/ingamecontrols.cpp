#include "ingamecontrols.h"

#include "resource.h"

#include <MyWidget/Layout>
#include "gen/ui_toppanel.h"
#include "formbuilder.h"
#include "button.h"
#include "scroolwidget.h"

#include "commandspanel.h"

#include "prototypesloader.h"
#include "listbox.h"
#include "font.h"
#include "algo/algo.h"
#include "game.h"

#include "ingamemenu.h"
#include "util/lexicalcast.h"

#include "unitlist.h"

#include <sstream>

using namespace MyWidget;

struct InGameControls::AddUnitButton: public Button{
  AddUnitButton( Resource & res, ProtoObject& obj )
      :Button(res), prototype(obj) {
    clicked.bind( *this, &AddUnitButton::click );
    Texture t;
    t.data = res.pixmap("gui/icon/"+obj.name);
    setText( obj.name );

    icon = t;
    }

  void click(){
    clickedEx( prototype );
    }

  MyWidget::signal<const ProtoObject&> clickedEx;
  ProtoObject& prototype;
  };

InGameControls::InGameControls(Resource &res,
                                BehaviorMSGQueue &msg,
                                PrototypesLoader &prototypes, Game &game)
               :res(res), game(game), prototypes(prototypes) {
  isHooksEnabled = true;
  currPl         = 1;
  setFocusPolicy( MyWidget::ClickFocus );

  //paintObjectsHud.bind( paintObjectsHud );
  frame.data = res.pixmap("gui/colors");

  setLayout( MyWidget::Vertical );
  layout().setMargin( MyWidget::Margin(4) );
  useScissor( false );

  Widget * top = new Widget(),
         * cen = new Widget();

  { SizePolicy p;
    p.typeV = MyWidget::Expanding;
    cen->setSizePolicy(p);
    }

  layout().add( top );
  layout().add( cen );

  layout().add( createConsole(msg) );

  top->setLayout( MyWidget::Horizontal );

  SizePolicy p;
  p.maxSize.h = 30;
  p.typeV = MyWidget::FixedMax;

  {
    UI::TopPanel p;
    p.setupUi( top, res );
    //p.fullScr->clicked.bind( toogleFullScreen );
    p.menu->clicked.bind( *this, &InGameControls::showMenu );
    p.frmEdit->clicked.bind( *this, &InGameControls::showFormBuilder );
    gold = p.gold;
    lim  = p.lim;

    gold->icon.data = res.pixmap("gui/icon/gold");
    lim-> icon.data = res.pixmap("gui/icon/house");
    }

  top->setSizePolicy(p);

  cen->layout().add( createEditPanel() );
  cen->useScissor( false );

  }

Widget *InGameControls::createConsole( BehaviorMSGQueue & q ) {
  Widget * console = new Widget();
  console->setMaximumSize( SizePolicy::maxWidgetSize().w, 220 );
  console->setMinimumSize( 0, 220);

  SizePolicy p;
  p.minSize = MyWidget::Size(220, 220);
  p.maxSize = MyWidget::Size(220, 220);

  console->setLayout( MyWidget::Horizontal );
  console->layout().setSpacing(6);

  Panel * img = new Panel( res );
  img->setSizePolicy(p);

  console->layout().add( img );

  Widget * cen = new Widget();
  cen->setLayout( MyWidget::Horizontal );

  Panel * cenp = new Panel( res );
  cenp->setMaximumSize( SizePolicy::maxWidgetSize().w, 220 );
  cenp->layout().setMargin(7);
  units = new UnitList(res);
  cenp->layout().add( units );

  cen->setSizePolicy( MyWidget::Expanding );

  MyWidget::Margin m(0);
  m.top = 50;
  cen->layout().setMargin(m);

  cen->layout().add( cenp );

  { Panel * avatar = new Panel(res);
    SizePolicy pa;
    pa.typeH = MyWidget::FixedMin;
    pa.minSize.w = 120;
    avatar->setSizePolicy( pa );

    cen->layout().add( avatar );
    }

  console->layout().add( cen );

  commands = new CommandsPanel(res, q);
  commands->onPageCanged.bind( *this, &InGameControls::removeAllHooks );

  commands->setSizePolicy( img->sizePolicy() );
  console->layout().add( commands );

  return console;
  }

Widget *InGameControls::createEditPanel() {
  Panel *p = new Panel(res);
  p->setDragable(1);
  p->layout().setMargin( 6, 6, 20, 6 );
  p->setLayout( MyWidget::Vertical );

  Button *btn = new Button(res);
  btn->clicked.bind( toogleEditLandMode );
  p->layout().add( btn );

  ScroolWidget *w = new ScroolWidget(res);
  //w->useScissor(0);

  auto proto = prototypes.allClasses();

  for( auto i=proto.begin(); i!=proto.end(); ++i ){
    bool ok = false;
    for( size_t r=0; !ok && r<(**i).view.size(); ++r )
      if( (**i).view[r].name.size() )
        ok = true;

    if( ok ){
      AddUnitButton *b = new AddUnitButton(res, **i);
      b->clickedEx.bind( *this, &InGameControls::addEditorObject );

      SizePolicy p = b->sizePolicy();
      p.typeH = MyWidget::Preferred;
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
  lbox->onItemSelected.bind( *this, &InGameControls::setCurrPl );
  lbox->setCurrentItem(currPl);

  p->layout().add( lbox );

  return p;
  }

void InGameControls::setCurrPl(size_t i) {
  currPl = i;
  }

void InGameControls::showFormBuilder() {
  new FormBuilder(res, this);
  }

void InGameControls::showMenu() {
  InGameMenu *m = new InGameMenu(res, this);

  m->save.bind( save );
  m->load.bind( load );
  }

void InGameControls::addEditorObject( const ProtoObject &p ) {
  addObject( p, currPl );
  }

void InGameControls::paintEvent(PaintEvent &e) {
  gold->setText( Lexical::upcast( game.player().gold() ) );
  int freeLim = game.player().limMax()-game.player().lim();
  lim-> setText( Lexical::upcast( freeLim ) +"/" +
                 Lexical::upcast( game.player().limMax() ) );

  Widget::paintEvent(e);

  MyWidget::Painter p(e);
  p.setTexture( frame );
  paintObjectsHud( p, w(), h() );

  p.setTexture( frame );
  MyWidget::Rect r = selection;
  if( r.w<0 ){
    r.x += r.w;
    r.w = -r.w;
    }
  if( r.h<0 ){
    r.y += r.h;
    r.h = -r.h;
    }
  int w = r.w, h = r.h;
  MyWidget::Rect tex = MyWidget::Rect(0,0, 1,1);

  p.setBlendMode( MyWidget::addBlend );
  p.drawRect( r, tex );

  r = MyWidget::Rect(r.x, r.y, 1, r.h);
  p.drawRect( r, tex );

  r = MyWidget::Rect(r.x+w-1, r.y, 1, r.h);
  p.drawRect( r, tex );

  r = MyWidget::Rect(r.x-w+2, r.y, w-2, 1);
  p.drawRect( r, tex );

  r = MyWidget::Rect(r.x, r.y+h-1, w-2, 1);
  p.drawRect( r, tex );

  p.unsetTexture();

  //p.setFont( mainFont );
  //p.drawText(100, 100, L"абвгд" );
  paintNested(e);
  }

void InGameControls::mouseDownEvent(MouseEvent &e) {
  e.ignore();
  hookCall( &InputHookBase::mouseDownEvent, e );
  }

void InGameControls::mouseUpEvent(MouseEvent &e) {
  e.ignore();
  hookCall( &InputHookBase::mouseUpEvent, e );
  }

void InGameControls::mouseMoveEvent(MouseEvent &e) {
  e.ignore();
  hookCall( &InputHookBase::mouseMoveEvent, e );
  }

void InGameControls::mouseWheelEvent(MouseEvent &e) {
  e.ignore();
  hookCall( &InputHookBase::mouseWheelEvent, e );
  }

void InGameControls::keyDownEvent(KeyEvent &e) {
  e.ignore();
  hookCall( &InputHookBase::keyDownEvent, e );
  }

void InGameControls::keyUpEvent(KeyEvent &e) {
  e.ignore();
  hookCall( &InputHookBase::keyUpEvent, e );
  }

bool InGameControls::instalHook(InputHookBase *h) {
  if( isHooksEnabled )
    hooks.push_back( h );

  return isHooksEnabled;
  }

void InGameControls::removeHook(InputHookBase *h) {
  h->onRemove();
  remove( hooks, h );
  }

void InGameControls::enableHooks(bool e) {
  if( isHooksEnabled != e ){
    isHooksEnabled = e;
    removeAllHooks();
    }
  }

void InGameControls::updateSelectUnits( const std::vector<GameObject *> &u ){
  removeAllHooks();
  commands->bind( u );

  units->setup( u );
  }

void InGameControls::onUnitDied(GameObject &obj) {
  units->onUnitDied( obj );
  }

void InGameControls::removeAllHooks() {
  for( size_t i=0; i<hooks.size(); ++i )
    hooks[i]->onRemove();
  hooks.clear();
  }
