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
#include "graphics/guipass.h"

#include "algo/algo.h"
#include "ingamecontrols.h"
#include "nativesavedialog.h"
#include "savedialog.h"

#include "gui/hintsys.h"
#include "lang/lang.h"

MainGui::MainGui( MyGL::Device &,
                  int w, int h,
                  Resource &r,
                  PrototypesLoader &pr )
  : res(r), prototypes(pr) {
  mainwidget = 0;
  central.resize(w,h);

  Font f, f2(14);
  std::wstring str;

  for( wchar_t i='a'; i<='z'; ++i )
    str.push_back(i);
  for( wchar_t i='A'; i<='Z'; ++i )
    str.push_back(i);

  for( wchar_t i='0'; i<='9'; ++i )
    str.push_back(i);

  str += L"/\\|!@#$%^&*()_-=+";

  f .fetch(res, str);
  f2.fetch(res, str);

  Lang::fetch(f2, res);
  }

MainGui::~MainGui() {
  delete mainwidget;
  }

void MainGui::setFocus() {
  mainwidget->setFocus(1);
  }

void MainGui::createControls(BehaviorMSGQueue & msg , Game &game) {  
  frame.data     = res.pixmap("gui/colors");
  hintFrame.data = res.pixmap("gui/hintFrame");

  mainwidget = new InGameControls(res, msg, prototypes, game);
  mainwidget->renderScene.bind( renderScene );
  mainwidget->setCameraPos.bind( setCameraPos );
  mainwidget->setCameraPosXY.bind( setCameraPosXY );

  updateView.bind( mainwidget->updateView );

  mainwidget->addObject.bind( addObject );
  mainwidget->toogleEditLandMode.bind( toogleEditLandMode );
  mainwidget->paintObjectsHud.bind( paintObjectsHud );

  mainwidget->save.bind( *this, &MainGui::saveGame );
  mainwidget->load.bind( *this, &MainGui::loadGame );

  central.layout().add( mainwidget );
  }

bool MainGui::draw(GUIPass &pass) {
  res.flushPixmaps();

  if( mainwidget ){
    mainwidget->updateValues();
    mainwidget->updateView();
    }

  {
    PainterGUI painter( pass, res, 0,0, central.w(), central.h() );
    MyWidget::PaintEvent event(painter);

    MyWidget::Painter p( event );
    pass.setCurrentBuffer(0);
    pass.clearBuffers();

    p.setTexture( frame );
    MyWidget::Rect r = selRect;
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

    paintObjectsHud( p, central.w(), central.h() );
  }

  if( central.needToUpdate() ){
    PainterGUI painter( pass, res, 0,0, central.w(), central.h() );
    MyWidget::PaintEvent event(painter);

    pass.setCurrentBuffer(1);
    pass.clearBuffers();

    central.paintEvent( event );
    }
  {
    PainterGUI painter( pass, res, 0,0, central.w(), central.h() );
    MyWidget::PaintEvent event(painter);

    MyWidget::Painter p( event );
    pass.setCurrentBuffer(2);
    pass.clearBuffers();

    if( HintSys::time>0 && HintSys::hint().size() ){
      if( !HintSys::vrect().contains(mousePos) )
        --HintSys::time;

      p.setFont( Font(14) );
      MyWidget::Size dpos = Font(14).textSize(res, HintSys::hint());
      dpos.w += 30;
      dpos.h += 30;

      MyWidget::Point pos = HintSys::pos() + MyWidget::Point( -dpos.w/2, -dpos.h );

      if( pos.y-dpos.h < 0 )
        pos.y += 2*dpos.h;

      pos.x = std::max(pos.x,0);
      pos.x = std::min(pos.x, central.w()-dpos.w);

      p.setTexture( hintFrame );
      MyWidget::Size s = hintFrame.data.rect.size();

      MyWidget::Rect tex = MyWidget::Rect(0, 0, s.w, s.h);
      p.setBlendMode( MyWidget::alphaBlend );
      int b = 10;

      p.drawRect( MyWidget::Rect(pos.x, pos.y, b,b),
                  MyWidget::Rect(0, 0, b, b) );
      p.drawRect( MyWidget::Rect(pos.x, pos.y+b, b, dpos.h-2*b),
                  MyWidget::Rect(0, b, b, tex.h-2*b) );
      p.drawRect( MyWidget::Rect(pos.x, pos.y+dpos.h-b, b, b),
                  MyWidget::Rect(0, tex.h-b, b, b) );

      p.drawRect( MyWidget::Rect(pos.x+dpos.w-b, pos.y, b,b),
                  MyWidget::Rect(s.w-b, 0, b, b) );
      p.drawRect( MyWidget::Rect(pos.x+dpos.w-b, pos.y+b, b, dpos.h-2*b),
                  MyWidget::Rect(s.w-b, b, b, tex.h-2*b) );
      p.drawRect( MyWidget::Rect(pos.x+dpos.w-b, pos.y+dpos.h-b, b, b),
                  MyWidget::Rect(s.w-b, tex.h-b, b, b) );

      p.drawRect( MyWidget::Rect(pos.x+b, pos.y, dpos.w-2*b, b ),
                  MyWidget::Rect(b, 0, tex.w-2*b, b ) );

      p.drawRect( MyWidget::Rect(pos.x+b, pos.y+dpos.h-b, dpos.w-2*b, b ),
                  MyWidget::Rect(b, s.h-b, tex.w-2*b, b ) );

      p.drawRect( MyWidget::Rect(pos.x+b, pos.y+b, dpos.w-2*b, dpos.h-2*b ),
                  MyWidget::Rect(b, b, tex.w-2*b, tex.h-2*b ) );

      //p.drawRect( MyWidget::Rect(pos.x, pos.y, dpos.w, dpos.h), tex );

      p.drawText( pos.x+15, pos.y+15, HintSys::hint() );
      }
    }

  return 1;
  }

void MainGui::resizeEvent(int w, int h) {
  central.resize( w, h );
  }

int MainGui::mouseDownEvent( MyWidget::MouseEvent &e ) {
  central.mouseDownEvent(e);
  return e.isAccepted();
  }

int MainGui::mouseUpEvent( MyWidget::MouseEvent &e) {
  central.mouseUpEvent(e);
  return e.isAccepted();
  }

int MainGui::mouseMoveEvent( MyWidget::MouseEvent &e) {
  mousePos = e.pos();

  central.mouseDragEvent(e);

  if( e.isAccepted() )
    return 1;

  central.mouseMoveEvent(e);

  return e.isAccepted();
  }

int MainGui::mouseWheelEvent(MyWidget::MouseEvent &e) {
  central.mouseWheelEvent(e);
  return e.isAccepted();
  }

int MainGui::scutEvent(MyWidget::KeyEvent &e) {
  e.ignore();
  central.shortcutEvent(e);
  return 1;
  }

int MainGui::keyDownEvent(MyWidget::KeyEvent &e) {
  e.ignore();
  central.keyDownEvent(e);
  return e.isAccepted();
  }

int MainGui::keyUpEvent(MyWidget::KeyEvent &e) {
  e.ignore();
  central.keyUpEvent(e);
  return e.isAccepted();
  }

MyWidget::Rect &MainGui::selectionRect() {
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

void MainGui::renderMinimap(World &w) {
  mainwidget->renderMinimap(w);
  }

void MainGui::updateValues() {
  MyWidget::CustomEvent e;
  mainwidget->customEvent(e);
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
  mainwidget->onUnitDied(obj);
  }
