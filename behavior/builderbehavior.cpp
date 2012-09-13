#include "builderbehavior.h"

#include "game/gameobject.h"
#include "game/world.h"
#include "game.h"

#include "graphics/glowmaterial.h"
#include "graphics/addmaterial.h"

#include "behavior/buildingbehavior.h"

BuilderBehavior::BuilderBehavior( GameObject & o,
                                  Behavior::Closure & c )
                : obj(o), hud(o.getScene()) {
  instaled = 0;
  hud.setVisible(0);

  blue = obj.game().resources().texture("glow/blue");
  red  = obj.game().resources().texture("glow/red" );

  hook.mouseUp  .bind( *this, &BuilderBehavior::mouseUp      );
  hook.mouseDown.bind( *this, &BuilderBehavior::mouseDown    );
  hook.mouseMove.bind( *this, &BuilderBehavior::mouseMove    );
  hook.onRemove .bind( *this, &BuilderBehavior::onRemoveHook );
  }

BuilderBehavior::~BuilderBehavior() {
  if( instaled )
    obj.game().removeHook( &hook );
  }

void BuilderBehavior::tick(const Terrain &) {
  if( tasks.size()==0 )
    return;

  CreateOrder & ord = tasks[0];

  if( !obj.isOnMove() ){
    if( obj.distanceQL( ord.x, ord.y )<=1 ){
      build();
      //taget.clear();
      } else
      move( ord.x, ord.y );
    }

  }

void BuilderBehavior::move(int x, int y) {
  int qs = Terrain::quadSize;

  x /= qs;
  y /= qs;
  obj.behavior.message( MoveSingle, x*qs + qs/2, y*qs + qs/2 );
  }

void BuilderBehavior::build() {
  CreateOrder & ord = tasks[0];

  GameObject& n = obj.world().addObject( ord.taget, obj.playerNum() );
  n.setPosition( ord.x,
                 ord.y,
                 0 );
  n.behavior.add( "incomplete" );

  tasks.erase( tasks.begin() );
  }

bool BuilderBehavior::message( AbstractBehavior::Message msg,
                               const std::string & s,
                               AbstractBehavior::Modifers md ) {
  if( msg!=Buy ){
    taget.clear();
    return 0;
    }

  if( !instaled )
    instaled = obj.game().instalHook( &hook );

  if( instaled ){
    taget = s;
    proto = &obj.game().prototype(taget);

    hud.setModel( obj.game().resources().model( proto->view[0].name+"/model" ) );
    obj.game().setupMaterials( hud, proto->view[0], obj.teamColor() );

    GameObject::setViewSize( hud, proto->view[0], 1, 1, 1 );
    GameObject::setViewPosition( hud,
                                 proto->view[0],
                                 World::coordCast(obj.world().mouseX()),
                                 World::coordCast(obj.world().mouseY()),
                                 0);

    hud.setRotation(0, 180);

    GlowMaterial mat;
    mat.texture = blue;
    hud.setupMaterial(mat);

    hud.setVisible(1);
    }

  //instaled = 1;
  return 1;
  }

bool BuilderBehavior::message( AbstractBehavior::Message msg,
                               int x, int y,
                               AbstractBehavior::Modifers /*md*/) {
  if( msg==Move ||
      msg==MoveGroup ||
      msg==MineralMove ){
    tasks.clear();
    }

  if( msg==BuildAt && taget.size()!=0 ){
    CreateOrder ord;
    ord.taget = taget;
    ord.x = x;
    ord.y = y;

    tasks.resize(1);
    tasks[0] = ord;

    move( tasks[0].x, tasks[0].y );
    }

  return 0;
  }

void BuilderBehavior::mouseDown(MyWidget::MouseEvent &e) {
  e.accept();
  }

void BuilderBehavior::mouseUp(MyWidget::MouseEvent & e ) {
  if( BuildingBehavior::canBuild( obj.world().terrain(),
                                  obj.game().prototype(taget),
                                  obj.world().mouseX(),
                                  obj.world().mouseY() )
      && e.button==MyWidget::Event::ButtonLeft ){
    onRemoveHook();
    obj.game().removeHook( &hook );

    obj.game().message( obj.playerNum(),
                        BuildAt,
                        obj.world().mouseX(),
                        obj.world().mouseY() );
    }

  if( e.button==MyWidget::Event::ButtonRight ){
    onRemoveHook();
    taget.clear();
    obj.game().removeHook( &hook );
    }
  }

void BuilderBehavior::mouseMove(MyWidget::MouseEvent &e) {
  GameObject::setViewPosition( hud,
                               proto->view[0],
                               World::coordCast(obj.world().mouseX()),
                               World::coordCast(obj.world().mouseY()),
                               0);

  bool cl =  BuildingBehavior::canBuild( obj.world().terrain(),
                                         obj.game().prototype(taget),
                                         obj.world().mouseX(),
                                         obj.world().mouseY() );
  GlowMaterial mat;
  mat.texture = cl ? blue : red;
  hud.setupMaterial(mat);

  e.ignore();
  }

void BuilderBehavior::onRemoveHook() {
  hud.setVisible(0);
  instaled = 0;
  }
