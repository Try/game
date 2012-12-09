#include "builderbehavior.h"

#include "game/gameobject.h"
#include "game/world.h"
#include "game.h"

#include "graphics/glowmaterial.h"
#include "graphics/addmaterial.h"

#include "graphics/mainmaterial.h"
#include "graphics/transparentmaterial.h"

#include "behavior/buildingbehavior.h"

BuilderBehavior::BuilderBehavior( GameObject & o,
                                  Behavior::Closure & c )
                : obj(o), hud(o.getScene()), hudIntent(o.getScene()) {
  instaled = 0;
  hud.setVisible(0);
  hudIntent.setVisible(0);

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

  const ProtoObject & proto = obj.game().prototype(ord.taget);

  bool landEnable = BuildingBehavior::canBuild( obj.world().terrain(),
                                                proto,
                                                ord.x,
                                                ord.y );

  if( landEnable ){
    GameObject& n = obj.world().addObject( ord.taget, obj.playerNum() );
    n.setPosition( ord.x,
                   ord.y,
                   0 );
    n.behavior.add( "incomplete" );

    //obj.player().addGold( -proto.data.gold );
    obj.player().addLim ( proto.data.lim );
    }

  tasks.erase( tasks.begin() );
  hudIntent.setVisible(0);
  }

bool BuilderBehavior::message( AbstractBehavior::Message msg,
                               const std::string & s,
                               AbstractBehavior::Modifers md ) {
  if( msg!=Buy ){
    taget.clear();
    return 0;
    }

  if( !instaled ){
    const ProtoObject & proto = obj.game().prototype(s);
    if( !obj.player().canBuild( proto ) )
      return 0;

    //obj.player().addGold( -proto.data.gold );
    //obj.player().addLim ( -proto.data.lim );
    instaled = obj.game().instalHook( &hook );
    }

  if( instaled ){
    taget = s;
    proto = &obj.game().prototype(taget);

    MyGL::Model<> m = obj.game().resources().model( proto->view[0].name+"/model" );
    hud.setModel( m );

    obj.game().setupMaterials( hud,       proto->view[0], obj.teamColor() );
    obj.game().setupMaterials( hudIntent, proto->view[0], obj.teamColor() );

    GameObject::setViewSize( hud, proto->view[0], 1, 1, 1 );
    GameObject::setViewSize( hudIntent, proto->view[0], 1, 1, 1 );

    GameObject::setViewPosition( hud,
                                 proto->view[0],
                                 World::coordCast(obj.world().mouseX()),
                                 World::coordCast(obj.world().mouseY()),
                                 0);

    hud.setRotation(0, 180);
    hudIntent.setRotation(0, 180);

    GlowMaterial mat;
    mat.texture = blue;
    hud.setupMaterial(mat);

    hud.setVisible(1);

    {
      hudIntent.unsetMaterial<MainMaterial>();

      TransparentMaterial mat( obj.game().shadowMat() );
      TransparentMaterialZPass zpass;
      //AddMaterial mat;
      mat.texture   = obj.game().resources().texture("util/blue");
      zpass.texture = mat.texture;

      hudIntent.setupMaterial(mat);
      hudIntent.setupMaterial(zpass);
      }

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
    if( tasks.size() ){
      obj.player().addGold( proto->data.gold );
      obj.player().addLim ( proto->data.lim );
      tasks.clear();
      hudIntent.setVisible(0);
      }
    }

  if( msg==BuildAt && taget.size()!=0 ){
    if( tasks.size() ){
      obj.player().addGold( proto->data.gold );
      obj.player().addLim ( proto->data.lim );
      tasks.clear();
      }

    CreateOrder ord;
    ord.taget = taget;
    ord.x = x;
    ord.y = y;

    tasks.resize(1);
    tasks[0] = ord;

    move( tasks[0].x, tasks[0].y );
    /*
    hudIntent.setPosition( World::coordCast(ord.x),
                           World::coordCast(ord.y),
                           0 );
                           */
    hudIntent.setVisible(1);
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
                                  obj.world().mouseY() ) &&
      e.button==MyWidget::Event::ButtonLeft &&
      obj.player().canBuild( *proto ) ){
    onRemoveHook();
    obj.game().removeHook( &hook );

    obj.player().addGold( -proto->data.gold );
    obj.player().addLim ( -proto->data.lim );

    obj.game().message( obj.playerNum(),
                        BuildAt,
                        obj.world().mouseX(),
                        obj.world().mouseY() );    

    MyGL::Model<>m = obj.game().resources().model(proto->view[0].name+"/model");
    hudIntent.setModel( m );

    GameObject::setViewPosition( hudIntent,
                                 proto->view[0],
                                 World::coordCast(obj.world().mouseX()),
                                 World::coordCast(obj.world().mouseY()),
                                 0);
    }

  if( e.button==MyWidget::Event::ButtonRight ){
    onRemoveHook();
    obj.player().addGold( proto->data.gold );
    obj.player().addLim ( proto->data.lim );
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
  //const ProtoObject & proto = obj.game().prototype(taget);
  hud.setVisible(0);
  instaled = 0;
  }
