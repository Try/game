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
#include "ingamecontrols.h"

MainGui::MainGui( MyGL::Device &,
                  int w, int h,
                  Resource &r,
                  PrototypesLoader &pr )
  : res(r), prototypes(pr) {
  mainwidget = 0;
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

void MainGui::setFocus() {
  mainwidget->setFocus(1);
  }

void MainGui::createControls(BehaviorMSGQueue & msg , Game &game) {
  mainwidget = new InGameControls(res, msg, prototypes, game);

  mainwidget->addObject.bind( addObject );
  mainwidget->toogleEditLandMode.bind( toogleEditLandMode );

  central.layout().add( mainwidget );
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
  return e.isAccepted();
  }

int MainGui::mouseUpEvent( MyWidget::MouseEvent &e) {
  central.mouseUpEvent(e);
  return e.isAccepted();
  }

int MainGui::mouseMoveEvent( MyWidget::MouseEvent &e) {
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
  central.keyUpEvent(e);
  return e.isAccepted();
  }

MyWidget::Rect &MainGui::selectionRect() {
  return mainwidget->selection;
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

void MainGui::onUnitDied(GameObject &obj) {
  mainwidget->onUnitDied(obj);
  }
