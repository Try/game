#include "maingui.h"

#include "graphics/paintergui.h"
#include "prototypesloader.h"
#include <Tempest/Event>

#include <Tempest/Layout>
#include <Tempest/Pixmap>

#include "gui/button.h"
#include "gui/panel.h"
#include "gui/scroolbar.h"
#include "gui/scroolwidget.h"
#include "gui/commandspanel.h"
#include "gui/overlaywidget.h"
#include "gui/listbox.h"
#include "graphics/guipass.h"

#include "algo/algo.h"
#include "ingamecontrols.h"
#include "nativesavedialog.h"
#include "savedialog.h"

#include "gui/hintsys.h"
#include "lang/lang.h"

#include "gui/richtext.h"

#include "graphics/paintergui.h"
#include "game.h"

MainGui::MainGui( Tempest::Device &,
                  int w, int h,
                  Resource &r,
                  PrototypesLoader &pr )
  : res(r), prototypes(pr) {
  mainwidget = 0;
  fps = -1;
  central.resize(w,h);

  std::wstring str;

  for( wchar_t i='a'; i<='z'; ++i )
    str.push_back(i);
  for( wchar_t i='A'; i<='Z'; ++i )
    str.push_back(i);

  for( wchar_t i='0'; i<='9'; ++i )
    str.push_back(i);

  str += L"/\\|!@#$%^&*()_-=+";

  for( int i=10; i<=16; ++i ){
    Font f(i);

    for( int bold = 0; bold<=1; ++bold )
      for( int italic=0; italic<=1; ++italic ){
        f.setBold(bold);
        f.setItalic(italic);

        f .fetch(res, str);
        Lang::fetch(f,  res);
        }
    }
  }

MainGui::~MainGui() {
  delete mainwidget;
  }

void MainGui::setFocus() {
  mainwidget->setFocus(1);
  }

void MainGui::createControls(BehaviorMSGQueue & msg, Game &game) {
  delete mainwidget;

  frame.data     = res.pixmap("gui/colors");
  hintFrame.data = res.pixmap("gui/hintFrame");
  game.updateMissionTargets = Tempest::signal<>();

  mainwidget = new InGameControls(res, msg, prototypes, game);
  mainwidget->renderScene. bind( renderScene  );
  mainwidget->setCameraPos.bind( setCameraPos );
  mainwidget->minimapEvent.bind( minimapEvent );

  mainwidget->onSettingsChanged.bind( onSettingsChanged );

  //updateView.bind( mainwidget->updateView );

  mainwidget->addObject.bind( addObject );
  mainwidget->onSetPlayer.bind( onSetPlayer );
  mainwidget->toogleEditTab.bind( toogleEditTab );

  mainwidget->toogleEditLandMode.bind( toogleEditLandMode );
  mainwidget->paintObjectsHud.bind( paintObjectsHud );

  mainwidget->save.bind( *this, &MainGui::saveGame );
  mainwidget->load.bind( *this, &MainGui::loadGame );

  central.layout().add( mainwidget );
  }

bool MainGui::draw(GUIPass &pass) {
  res.flushPixmaps();  

  if( mainwidget ){
    updateValues();
    mainwidget->updateValues();
    mainwidget->updateView();
    updateView();
    }

  {
    PainterGUI painter( pass, res, 0,0, central.w(), central.h() );
    Tempest::PaintEvent event(painter);

    Tempest::Painter p( event );
    pass.setCurrentBuffer(0);
    pass.clearBuffers();

    p.setTexture( frame );
    Tempest::Rect r = selRect;
    if( r.w<0 ){
      r.x += r.w;
      r.w = -r.w;
      }
    if( r.h<0 ){
      r.y += r.h;
      r.h = -r.h;
      }
    int w = r.w, h = r.h;
    Tempest::Rect tex = Tempest::Rect(0,0, 1,1);

    p.setBlendMode( Tempest::addBlend );
    p.drawRect( r, tex );

    r = Tempest::Rect(r.x, r.y, 1, r.h);
    p.drawRect( r, tex );

    r = Tempest::Rect(r.x+w-1, r.y, 1, r.h);
    p.drawRect( r, tex );

    r = Tempest::Rect(r.x-w+2, r.y, w-2, 1);
    p.drawRect( r, tex );

    r = Tempest::Rect(r.x, r.y+h-1, w-2, 1);
    p.drawRect( r, tex );

    p.unsetTexture();

    paintObjectsHud( p, central.w(), central.h() );

    if( isCutsceneMode() ){
      p.setTexture( frame );
      Tempest::Rect tex = Tempest::Rect(4,0, 1,1);

      p.setBlendMode( Tempest::noBlend );
      p.drawRect( Tempest::Rect(0,0, central.w(), 80), tex );
      p.drawRect( Tempest::Rect(0, central.h()-80, central.w(), 80), tex );

      p.unsetTexture();
      }
  }

  if( central.needToUpdate() ){
    PainterGUI painter( pass, res, 0,0, central.w(), central.h() );
    Tempest::PaintEvent event(painter);

    pass.setCurrentBuffer(1);
    pass.clearBuffers();

    central.paintEvent( event );
    }
  {
    PainterGUI painter( pass, res, 0,0, central.w(), central.h() );
    Tempest::PaintEvent event(painter);

    Tempest::Painter p( event );
    pass.setCurrentBuffer(2);
    pass.clearBuffers();

    if( HintSys::time>0 && HintSys::hint().size() ){
      if( !HintSys::vrect().contains(mousePos) )
        --HintSys::time;

      p.setFont( Font(14) );
      Tempest::Size dpos = RichText::bounds( res, HintSys::hint() );
      //Font(14).textSize(res, HintSys::hint());
      dpos.w += 30;
      dpos.h += 30;

      Tempest::Point pos = HintSys::pos() + Tempest::Point( -dpos.w/2, -dpos.h );

      if( pos.y-dpos.h < 0 )
        pos.y += 2*dpos.h;

      pos.x = std::max(pos.x,0);
      pos.x = std::min(pos.x, central.w()-dpos.w);

      p.setTexture( hintFrame );
      p.setBlendMode( Tempest::alphaBlend );

      MainGui::drawFrame( p, hintFrame, pos, dpos );
      RichText::renderText( pos.x+15, pos.y+15, res, p, HintSys::hint() );
      //p.drawText( pos.x+15, pos.y+15, HintSys::hint() );
      }

    if( fps>=0 ){
      p.setFont( Font(14) );
      RichText::renderText( 0, 0, res, p, fpsStr);
      }
    }

  return 1;
  }

void MainGui::resizeEvent(int w, int h) {
  central.resize( w, h );
  }

int MainGui::mouseDownEvent( Tempest::MouseEvent &e ) {
  central.mouseDownEvent(e);
  return e.isAccepted();
  }

int MainGui::mouseUpEvent( Tempest::MouseEvent &e) {
  central.mouseUpEvent(e);
  return e.isAccepted();
  }

int MainGui::mouseMoveEvent( Tempest::MouseEvent &e) {
  mousePos = e.pos();

  central.mouseDragEvent(e);

  if( e.isAccepted() )
    return 1;

  central.mouseMoveEvent(e);

  return e.isAccepted();
  }

int MainGui::mouseWheelEvent(Tempest::MouseEvent &e) {
  central.mouseWheelEvent(e);
  return e.isAccepted();
  }

int MainGui::scutEvent(Tempest::KeyEvent &e) {
  e.ignore();
  central.shortcutEvent(e);
  return 1;
  }

int MainGui::keyDownEvent(Tempest::KeyEvent &e) {
  e.ignore();
  central.keyDownEvent(e);
  return e.isAccepted();
  }

int MainGui::keyUpEvent(Tempest::KeyEvent &e) {
  e.ignore();
  central.keyUpEvent(e);
  return e.isAccepted();
  }

bool MainGui::minimapMouseEvent( float x, float y,
                                 Tempest::Event::MouseButton btn,
                                 MiniMapView::Mode m ) {
  return mainwidget->minimapMouseEvent(x,y,btn,m);
  }

Tempest::Rect &MainGui::selectionRect() {
  return selRect;
  }

void MainGui::update() {
  central.update();
  }

void MainGui::updateSelectUnits( const std::vector<GameObject *> &u ) {
  mainwidget->updateSelectUnits( u );
  }

bool MainGui::instalHook(InputHookBase *h) {
  return mainwidget->instalHook(h);
  }

void MainGui::removeHook(InputHookBase *h) {
  mainwidget->removeHook(h);
  }

void MainGui::enableHooks(bool e) {
  mainwidget->enableHooks(e);
  }

void MainGui::setupMinimap(World *w) {
  mainwidget->setupMinimap(w);
  }

void MainGui::renderMinimap() {
  mainwidget->renderMinimap();
  }

void MainGui::updateValues() {
  Tempest::CustomEvent e;
  central.customEvent(e);
  }

bool MainGui::isCutsceneMode() {
  return !mainwidget->isVisible();
  }

void MainGui::drawFrame( Tempest::Painter & p,
                         const Tempest::Bind::UserTexture &frame,
                         const Tempest::Point &pos,
                         const Tempest::Size &dpos ) {
  Tempest::Rect tex = Tempest::Rect( 0, 0,
                                       frame.data.rect.w,
                                       frame.data.rect.h);
  p.setBlendMode( Tempest::alphaBlend );
  p.setTexture( frame );
  int b = 10;

  Tempest::Size s = frame.data.rect.size();

  p.drawRect( Tempest::Rect(pos.x, pos.y, b,b),
              Tempest::Rect(0, 0, b, b) );
  p.drawRect( Tempest::Rect(pos.x, pos.y+b, b, dpos.h-2*b),
              Tempest::Rect(0, b, b, tex.h-2*b) );
  p.drawRect( Tempest::Rect(pos.x, pos.y+dpos.h-b, b, b),
              Tempest::Rect(0, tex.h-b, b, b) );

  p.drawRect( Tempest::Rect(pos.x+dpos.w-b, pos.y, b,b),
              Tempest::Rect(s.w-b, 0, b, b) );
  p.drawRect( Tempest::Rect(pos.x+dpos.w-b, pos.y+b, b, dpos.h-2*b),
              Tempest::Rect(s.w-b, b, b, tex.h-2*b) );
  p.drawRect( Tempest::Rect(pos.x+dpos.w-b, pos.y+dpos.h-b, b, b),
              Tempest::Rect(s.w-b, tex.h-b, b, b) );

  p.drawRect( Tempest::Rect(pos.x+b, pos.y, dpos.w-2*b, b ),
              Tempest::Rect(b, 0, tex.w-2*b, b ) );

  p.drawRect( Tempest::Rect(pos.x+b, pos.y+dpos.h-b, dpos.w-2*b, b ),
              Tempest::Rect(b, s.h-b, tex.w-2*b, b ) );

  p.drawRect( Tempest::Rect(pos.x+b, pos.y+b, dpos.w-2*b, dpos.h-2*b ),
              Tempest::Rect(b, b, tex.w-2*b, tex.h-2*b ) );
  }

MainGui::Widget *MainGui::centralWidget() {
  return mainwidget;
  }

void MainGui::setCutsceneMode(bool cs) {
  mainwidget->setVisible(cs==0);
  }

void MainGui::setFPS(float f) {
  if( fps!=f ){
    fpsStr = L"fps = "+Lexical::upcastw(f);
    fps = f;
    }
  }

void MainGui::saveGame() {
  SaveDialog* dlg = new SaveDialog(res, mainwidget);
  dlg->setSaveMode();
  dlg->accept.bind( save );
  }

void MainGui::loadGame() {
  SaveDialog* dlg = new SaveDialog(res, mainwidget);
  dlg->accept.bind( load );
  }

void MainGui::onUnitDied(GameObject &obj) {
  if( mainwidget )
    mainwidget->onUnitDied(obj);
  }
