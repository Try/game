#include "ingamecontrols.h"

#include "resource.h"

#include <Tempest/Layout>
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
#include "unitinfo.h"

#include "gui/textureview.h"
#include "gui/unitview.h"
#include "gui/minimapview.h"
#include "gui/lineedit.h"

#include "gui/tabwidget.h"
#include "editterrainpanel.h"
#include "missiontargets.h"

#include "graphicssettingswidget.h"

#include <sstream>

using namespace Tempest;

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

  Tempest::signal<const ProtoObject&> clickedEx;
  ProtoObject& prototype;
  };

InGameControls::InGameControls( Resource &res,
                                BehaviorMSGQueue &,
                                PrototypesLoader &prototypes, Game &game)
               : res(res),
                 game(game),
                 prototypes(prototypes)
                 //showEditPanel(this, Tempest::KeyEvent::K_F9),
                 //showSettings (this, Tempest::KeyEvent::K_F8)
  {
  isHooksEnabled = true;
  setFocusPolicy( Tempest::ClickFocus );

  game.scenario().setupUI(this, res);
  }

InGameControls::~InGameControls() {
  }

void InGameControls::showFormBuilder() {
  new FormBuilder(res, this);
  }

void InGameControls::showMenu() {
  game.pause(1);

  InGameMenu *m = new InGameMenu(res, this);

  m->save.bind( save );
  m->load.bind( load );
  m->onClosed.bind( game, &Game::unsetPause );
  m->quit.bind( game.exitGame );
  }

void InGameControls::setupMinimap(World *w) {
  game.scenario().setupMinimap(w);
  }

void InGameControls::renderMinimap() {
  game.scenario().renderMinimap();
  }

bool InGameControls::minimapMouseEvent( float x, float y,
                                        Event::MouseButton btn,
                                        MiniMapView::Mode m) {
  for( size_t i=0; i<hooks.size(); ++i ){
    InputHookBase &b = *hooks[hooks.size()-i-1];

    if( b.minimapMouseEvent(x,y,btn, m) )
      return 1;
    }

  return 0;
  }

void InGameControls::paintEvent(PaintEvent &e) {
  Widget::paintEvent(e);

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
  if( isHooksEnabled && hooks.size()==0 ){
    hooks.push_back( h );
    return 1;
    }

  return 0;
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
  game.scenario().updateSelectUnits(u);
  //units->setup( u );
  }

void InGameControls::onUnitDied(GameObject &) {
  //units->onUnitDied( obj );
  }

void InGameControls::removeAllHooks() {
  for( size_t i=0; i<hooks.size(); ++i )
    hooks[i]->onRemove();
  hooks.clear();
  }
