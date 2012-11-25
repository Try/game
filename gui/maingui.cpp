#include "maingui.h"

#include "graphics/paintergui.h"
#include "prototypesloader.h"
#include <MyWidget/Event>

#include <MyWidget/Layout>
#include <MyGL/Pixmap>

#include "gui/button.h"
#include "gui/panel.h"
#include "gui/scroolbar.h"
#include "gui/scroolwidget.h"
#include "gui/commandspanel.h"
#include "gui/overlaywidget.h"
#include "gui/listbox.h"

#include "algo/algo.h"
#include "formbuilder.h"

MainGui::MainGui( MyGL::Device &, int w, int h,
                  Resource &r, PrototypesLoader &pr )
  : res(r), prototypes(pr), isHooksEnabled(true) {
  mainwidget = new MainWidget();
  central.layout().add( mainwidget );
  central.resize(w,h);

  Font f;
  std::wstring str;

  for( wchar_t i='a'; i<'z'; ++i )
    str.push_back(i);
  for( wchar_t i='A'; i<'Z'; ++i )
    str.push_back(i);

  for( wchar_t i='0'; i<'9'; ++i )
    str.push_back(i);

  f.fetch(res, str);
  }

MainGui::~MainGui() {
  }

void MainGui::createControls( BehaviorMSGQueue & msg ) {
  MyWidget::Widget & widget = central.top();

  mainwidget->paintObjectsHud.bind( paintObjectsHud );
  mainwidget->frame.data = res.pixmap("gui/colors");

  widget.setLayout( MyWidget::Vertical );
  widget.layout().setMargin( MyWidget::Margin(4) );
  widget.useScissor( false );

  Widget * top = new Widget(),
         * cen = new Widget();

  { SizePolicy p;
    p.typeV = MyWidget::Expanding;
    cen->setSizePolicy(p);
    }

  widget.layout().add( top );
  widget.layout().add( cen );

  widget.layout().add( createConsole(msg) );

  top->setLayout( MyWidget::Horizontal );

  SizePolicy p;
  p.maxSize.h = 30;
  p.typeV = MyWidget::FixedMax;

  top->setSizePolicy(p);
  for( int i=0; i<3; ++i ){
    Button * b = new Button(res);
    top->layout().add( b );
    b->clicked.bind( toogleFullScreen );
    }
  top->layout().add( new Widget() );

  cen->layout().add( createEditPanel() );
  cen->layout().add( new FormBuilder(res) );

  cen->useScissor( false );
  }

MainGui::Widget *MainGui::createConsole( BehaviorMSGQueue & q ) {
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
  commands->onPageCanged.bind(*this, &MainGui::removeAllHooks );

  commands->setSizePolicy( img->sizePolicy() );
  console->layout().add( commands );

  return console;
  }

MainGui::Widget *MainGui::createEditPanel() {
  Panel *p = new Panel(res);
  p->setDragable(1);
  p->layout().setMargin( 6, 6, 20, 6 );
  p->setLayout( MyWidget::Vertical );

  ScroolWidget *w = new ScroolWidget(res);
  w->useScissor(0);

  auto proto = prototypes.allClasses();

  for( auto i=proto.begin(); i!=proto.end(); ++i ){
    bool ok = false;
    for( size_t r=0; !ok && r<(**i).view.size(); ++r )
      if( (**i).view[r].name.size() )
        ok = true;

    if( ok ){
      AddUnitButton *b = new AddUnitButton(res, **i);
      b->clickedEx.bind( addObject );

      SizePolicy p = b->sizePolicy();
      p.typeH = MyWidget::Preferred;
      b->setSizePolicy(p);

      w->centralWidget().layout().add( b );
      }
    }

  p->layout().add(w);
  p->layout().add( new ListBox(*this, res) );

  return p;
  }

bool MainGui::draw(GUIPass &pass) {
  res.flushPixmaps();

  if( central.needToUpdate() ){
    PainterGUI painter( pass, res, 0,0, central.w(), central.h() );
    MyWidget::PaintEvent event(painter);
    central.paintEvent( event );
    return 1;
    }

  return 0;
  }

void MainGui::resizeEvent(int w, int h) {
  central.resize( w, h );
  }

int MainGui::mouseDownEvent( MyWidget::MouseEvent &e ) {
  central.mouseDownEvent(e);

  if( hookCall( &InputHookBase::mouseDownEvent, e ) )
    return 1;

  return e.isAccepted();
  }

int MainGui::mouseUpEvent( MyWidget::MouseEvent &e) {
  central.mouseUpEvent(e);

  if( hookCall( &InputHookBase::mouseUpEvent, e ) )
    return 1;

  return e.isAccepted();
  }

int MainGui::mouseMoveEvent( MyWidget::MouseEvent &e) {
  central.mouseDragEvent(e);

  if( e.isAccepted() )
    return 1;

  central.mouseMoveEvent(e);

  if( hookCall( &InputHookBase::mouseMoveEvent, e ) )
    return 1;

  return e.isAccepted();
  }

int MainGui::mouseWheelEvent(MyWidget::MouseEvent &e) {
  central.mouseWheelEvent(e);

  if( hookCall( &InputHookBase::mouseWheelEvent, e ) )
    return 1;
  return e.isAccepted();
  }

int MainGui::keyDownEvent(MyWidget::KeyEvent &e) {
  e.ignore();
  central.shortcutEvent(e);

  if( e.isAccepted() )
    return 1;

  e.accept();
  central.keyDownEvent(e);

  if( hookCall( &InputHookBase::keyDownEvent, e ) )
    return 1;

  return e.isAccepted();
  }

int MainGui::keyUpEvent(MyWidget::KeyEvent &e) {
  //e.ignore();
  central.keyUpEvent(e);

  if( hookCall( &InputHookBase::keyUpEvent, e ) )
    return 1;

  return e.isAccepted();
  }

MyWidget::Rect &MainGui::selectionRect() {
  return mainwidget->selection;
  }

void MainGui::update() {
  central.update();
  }

void MainGui::updateSelectUnits( const std::vector<GameObject*> &u ) {
  removeAllHooks();
  commands->bind( u );
  }

bool MainGui::instalHook(InputHookBase *h) {
  if( isHooksEnabled )
    hooks.push_back( h );

  return isHooksEnabled;
  }

void MainGui::removeHook(InputHookBase *h) {
  h->onRemove();
  remove( hooks, h );
  }

void MainGui::enableHooks(bool e) {
  if( isHooksEnabled != e ){
    isHooksEnabled = e;
    removeAllHooks();
    }
  }

OverlayWidget *MainGui::addOverlay() {
  OverlayWidget * w = new OverlayWidget( *this, res );
  MyWidget::Widget * container = new MyWidget::Widget();

  container->setLayout( new OverlayWidget::ContainerLayout() );
  container->layout().add( w );

  if( central.layout().widgets().size()>1 )
    central.layout().widgets().back()->layout().add( container ); else
    central.layout().add( container );

  w->setFocus(1);
  w->setupSignals();

  return w;
  }

void MainGui::removeAllHooks() {
  for( size_t i=0; i<hooks.size(); ++i )
    hooks[i]->onRemove();
  hooks.clear();
  }

MainGui::AddUnitButton::AddUnitButton(Resource &res, ProtoObject &obj)
  :Button(res), prototype(obj) {
  clicked.bind( *this, &AddUnitButton::click );
  Texture t;
  t.data = res.pixmap("gui/icon/"+obj.name);
  setText( obj.name );

  Font f;
  f.fetch(res, txt);

  icon = t;
  }

void MainGui::AddUnitButton::click() {
  clickedEx( prototype );
  }

void MainGui::MainWidget::paintEvent( MyWidget::PaintEvent &e ) {
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

