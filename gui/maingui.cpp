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

#include "algo/algo.h"

MainGui::MainGui( MyGL::Device &, int w, int h,
                  Resource &r, PrototypesLoader &pr )
  : res(r), prototypes(pr), isHooksEnabled(true) {
  //painter = new PainterGUI();
  widget.resize(w,h);
  }

MainGui::~MainGui() {
  }

void MainGui::createControls( BehaviorMSGQueue & msg ) {
  widget.paintObjectsHud.bind( paintObjectsHud );
  widget.frame.data = res.pixmap("gui/colors");

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

  ScroolWidget *w = new ScroolWidget(res);

  auto proto = prototypes.allClasses();

  for( auto i=proto.begin(); i!=proto.end(); ++i ){
    bool ok = false;
    for( size_t r=0; !ok && r<(**i).view.size(); ++r )
      if( (**i).view[r].name.size() )
        ok = true;

    if( ok ){
      AddUnitButton *b = new AddUnitButton(res, **i);
      b->clickedEx.bind( addObject, &MyWidget::signal<const ProtoObject&>::operator() );

      SizePolicy p = b->sizePolicy();
      p.typeH = MyWidget::Preferred;
      b->setSizePolicy(p);

      w->centralWidget().layout().add( b );
      }
    }

  p->layout().add(w);

  return p;
  }

bool MainGui::draw(GUIPass &pass) {
  if( !widget.needToUpdate() )
    return false;

  PainterGUI painter( pass, 0,0, widget.w(), widget.h() );

  MyWidget::PaintEvent event(painter);
  widget.paintEvent( event );

  return 1;
  }

void MainGui::resizeEvent(int w, int h) {
  widget.resize( w, h );
  }

int MainGui::mouseDownEvent( MyWidget::MouseEvent &e ) {
  widget.mouseDownEvent(e);

  if( hookCall( &InputHookBase::mouseDownEvent, e ) )
    return 1;

  return e.isAccepted();
  }

int MainGui::mouseUpEvent( MyWidget::MouseEvent &e) {
  widget.mouseUpEvent(e);
  if( hookCall( &InputHookBase::mouseUpEvent, e ) )
    return 1;

  return e.isAccepted();
  }

int MainGui::mouseMoveEvent( MyWidget::MouseEvent &e) {
  widget.mouseDragEvent(e);
  if( e.isAccepted() )
    return 1;

  widget.mouseMoveEvent(e);

  if( hookCall( &InputHookBase::mouseMoveEvent, e ) )
    return 1;

  return e.isAccepted();
  }

int MainGui::mouseWheelEvent(MyWidget::MouseEvent &e) {
  widget.mouseWheelEvent(e);

  if( hookCall( &InputHookBase::mouseWheelEvent, e ) )
    return 1;
  return e.isAccepted();
  }

int MainGui::keyDownEvent(MyWidget::KeyEvent &e) {
  e.ignore();
  widget.shortcutEvent(e);
  if( e.isAccepted() )
    return 1;

  widget.keyDownEvent(e);

  if( hookCall( &InputHookBase::keyDownEvent, e ) )
    return 1;

  return e.isAccepted();
  }

int MainGui::keyUpEvent(MyWidget::KeyEvent &e) {
  widget.keyUpEvent(e);

  if( hookCall( &InputHookBase::keyUpEvent, e ) )
    return 1;

  return e.isAccepted();
  }

MyWidget::Rect &MainGui::selectionRect() {
  return widget.selection;
  }

void MainGui::update() {
  widget.update();
  }

void MainGui::updateSelectUnits( const std::vector<GameObject*> &u ) {
  removeAllHooks();
  commands->bind( u );
  }

bool MainGui::instalHook(InputHookBase *h) {
  if( isHooksEnabled )
    widget.hooks.push_back( h );

  return isHooksEnabled;
  }

void MainGui::removeHook(InputHookBase *h) {
  h->onRemove();
  remove( widget.hooks, h );
  }

void MainGui::enableHooks(bool e) {
  if( isHooksEnabled != e ){
    isHooksEnabled = e;
    removeAllHooks();
    }
  }

void MainGui::removeAllHooks() {
  for( size_t i=0; i<widget.hooks.size(); ++i )
    widget.hooks[i]->onRemove();
  widget.hooks.clear();
  }

MainGui::AddUnitButton::AddUnitButton(Resource &res, ProtoObject &obj)
  :Button(res), prototype(obj) {
  clicked.bind( *this, &AddUnitButton::click );
  Texture t;
  t.data = res.pixmap("gui/icon/"+obj.name);

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

  paintNested(e);
  }

