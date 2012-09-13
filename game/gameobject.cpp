#include "gameobject.h"

#include <MyGL/GraphicObject>
#include <MyGL/Algo/GBufferFillPass>

#include "resource.h"

#include "game.h"

#include "game/world.h"
#include "prototypesloader.h"

#include "algo/algo.h"
#include <cmath>
#include "game/player.h"

GameObject::GameObject( MyGL::Scene & s,
                        World &w,
                        const ProtoObject &p,
                        const PrototypesLoader &pl )
  : scene(s),
    wrld(w),
    prototypes(pl),
    selection(s),
    myClass(&p) {
  m.x = 0;
  m.y = 0;
  m.z = 0;

  m.pl = 0;
  wrld.player( m.pl ).addUnit(this);
  m.teamColor = wrld.player( m.pl ).color();

  m.intentDirX = 1;
  m.intentDirY = 0;

  m.isSelected = false;

  m.radius = 0;
  setMouseOverFlag(0);

  behavior.bind( *this, bclos );

  for( auto i=p.behaviors.begin(); i!=p.behaviors.end(); ++i ){
    behavior.add( *i );
    }
  }

GameObject::~GameObject() {
  wrld.player( m.pl ).delUnit(this);
  freePhysic();
  }

int GameObject::distanceSQ(const GameObject &other) const {
  return distanceSQ( other.x(), other.y() );
  }

int GameObject::distanceSQ(int xx, int xy) const {
  int dx = x() - xx,
      dy = y() - xy;

  return dx*dx + dy*dy;
  }

int GameObject::distanceQ(int xx, int yy) const {
  int qs = Terrain::quadSize;
  int dx = x()/qs - xx/qs,
      dy = y()/qs - yy/qs;

  return abs(dx) + abs(dy);
  }

int GameObject::distanceQL(int xx, int yy) const {
  int qs = Terrain::quadSize;
  int dx = x()/qs - xx/qs,
      dy = y()/qs - yy/qs;

  return std::max( abs(dx), abs(dy) );
  }

void GameObject::freePhysic(){
  if( physic ){
    physic->free(form.sphere);

    physic->free(anim.box);
    physic->free(anim.sphere);
    }
  }

void GameObject::setupMaterials( MyGL::AbstractGraphicObject &obj,
                                 const ProtoObject::View &src ) {
  game().setupMaterials( obj, src, teamColor() );
  }

void GameObject::loadView( Resource &r, Physics & p, bool env ) {
  physic = &p;
  view.clear();
  m.radius = 0;

  if( env )
    behavior.clear();

  for( size_t i=0; i<getClass().view.size(); ++i ){
    const ProtoObject::View &v = getClass().view[i];

    loadView( r, v, env );

    if( v.physModel==ProtoObject::View::Sphere ){
      if( env ){
        setForm( p.createSphere( x(), y(), 0,
                                 getClass().view[i].sphereDiameter ) );
        } else {
        setForm( p.createAnimatedSphere
                 ( x(), y(), 0, getClass().view[i].sphereDiameter ));
        }
      }

    if( v.physModel==ProtoObject::View::Box ){
      if( env ){
        /////////////
        } else {
        const double *bs = getClass().view[i].boxSize;
        setForm( p.createAnimatedBox( x(), y(), 0, bs[0], bs[1], bs[2] ));
        }
      }
    }

  const ProtoObject::View & v = prototypes.get("selection.green").view[0];
  setupMaterials( selection, v );
  selection.setModel( r.model("quad/model") );

  setViewSize(1);
  rotate( 180 );
  }

void GameObject::loadView( Resource & r,
                           const ProtoObject::View &src,
                           bool isEnv ) {
  const MyGL::Model<> & model = r.model( src.name+"/model" );

  for( int i=0; i<3; ++i )
    m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];

  MyGL::AbstractGraphicObject * obj = 0;

  if( isEnv ){
    EnvObject object( scene );    
    object.setModel( model );

    env.push_back( object );
    obj = &env.back();
    } else {
    MyGL::GraphicObject object( scene );
    object.setModel( model );

    view.push_back( object );
    obj = &view.back();

    if( src.randRotate )
      view.back().setRotation(0, rand()%360 );
    }

  setupMaterials(*obj, src );

  m.radius = std::max(m.radius, model.bounds().diameter()/2.0 );
  }

void GameObject::loadView( const MyGL::Model<> &model ){
  view.clear();

  for( int i=0; i<3; ++i )
    m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];

  MyGL::GraphicObject object( scene );
  object.setModel( model );
  m.radius = model.bounds().radius();

  setupMaterials( object, getClass().view[0] );
  view.push_back( object );
  }

void GameObject::loadView( const MyGL::Model<Terrain::WVertex> &model ){
  view.clear();

  for( int i=0; i<3; ++i )
    m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];

  MyGL::GraphicObject object( scene );
  object.setModel( model );
  m.radius = model.bounds().radius();

  setupMaterials( object, getClass().view[0] );
  view.push_back( object );
  }

void GameObject::setViewPosition( MyGL::GraphicObject& obj,
                                  const ProtoObject::View & v,
                                  float x,
                                  float y, float z) {
  float dx = 0, dy = 0, dz = 0;
  double modelSize[3] = { obj.bounds().max[0] - obj.bounds().min[0],
                          obj.bounds().max[1] - obj.bounds().min[1],
                          obj.bounds().max[2] - obj.bounds().min[2] };

  const int * align = v.align;
  double alignSize  = v.alignSize;

  dx = modelSize[0]*obj.sizeX()*align[0]*alignSize;
  dy = modelSize[1]*obj.sizeY()*align[1]*alignSize;
  dz = modelSize[2]*obj.sizeZ()*align[2]*alignSize;

  obj.setPosition( x+dx, y+dy, z+dz );
  }

void GameObject::setViewPosition(float x, float y, float z) {
  for( size_t i=0; i<view.size(); ++i ){
    setViewPosition( view[i], getClass().view[i], x, y, z );
    }

  selection.setPosition( x, y, z+0.01 );

  if( form.sphere.isValid() )
    form.sphere.setPosition(x,y,z);

  float dx = 0, dy = 0, dz = 0;
  if( view.size() ){
    const int * align = getClass().view[0].align;
    double alignSize  = getClass().view[0].alignSize;

    dx = m.modelSize[0]*view[0].sizeX()*align[0]*alignSize;
    dy = m.modelSize[1]*view[0].sizeY()*align[1]*alignSize;
    dz = m.modelSize[2]*view[0].sizeZ()*align[2]*alignSize;
    }

  if( anim.sphere.isValid() ){
    anim.sphere.setPosition(x+dx,y+dy,z+dz);
    }
  }

double GameObject::viewHeight() const {
  if( view.size() ){
    const int   align = getClass().view[0].align[2];
    double alignSize  = getClass().view[0].alignSize;

    return m.modelSize[2]*view[0].sizeZ()*( 1 - align*alignSize );
    }

  return 0;
  }

const MyGL::Color& GameObject::teamColor() const {
  return m.teamColor;
  }

void GameObject::setPosition(int x, int y, int z) {
  setViewPosition( World::coordCast(x),
                   World::coordCast(y),
                   World::coordCast(z) );
  m.x = x;
  m.y = y;
  m.z = z;//world.terrain().heightAt(x, y);

  behavior.message( Behavior::onPositionChange, x, y );
  }

void GameObject::setViewSize(float s) {
  setViewSize(s,s,s);
  }

void GameObject::setViewSize( MyGL::GraphicObject &obj,
                              const ProtoObject::View & v,
                              float x, float y, float z ) {
  const double *s = v.size;
  obj.setSize(s[0]*x, s[1]*y, s[2]*z);
  }

void GameObject::setViewSize(float x, float y, float z) {
  for( size_t i=0; i<view.size(); ++i ){
    setViewSize( view[i], getClass().view[i], x, y, z );
    }

  double ss[3] = { m.radius*x, m.radius*y, z };

  m.selectionSize[2] = ss[2];
  if( view.size() ){
    for( int i=0; i<2; ++i )
      m.selectionSize[i] = ss[i]*getClass().view[0].size[i]/1.44;
    }

  selection.setVisible( m.isMouseOwer );

  double s = m.selectionSize[0];
  for( int i=0; i<2; ++i )
    s = std::min(s, m.selectionSize[i] );

  selection.setSize( s );
  }

void GameObject::setForm(const Physics::Sphere &f) {
  form.sphere = f;
  //animForm = Physics::AnimatedSphere();
  }

void GameObject::setForm(const Physics::AnimatedSphere &f) {
  anim.sphere = f;
  //form = Physics::Sphere();
  }

void GameObject::setForm(const Physics::AnimatedBox &f) {
  anim.box = f;
  //form = Physics::Sphere();
  }

void GameObject::updatePos() {
  //anim bodyes
  if( anim.sphere.isValid() && view.size() ){
    MyGL::GraphicObject & g = view[0];
    anim.sphere.setPosition( g.x(), g.y(), g.z() );
    }

  if( anim.box.isValid() && view.size() ){
    MyGL::GraphicObject & g = view[0];
    anim.box.setPosition( g.x(), g.y(), g.z() );
    }

  //rigid bodyes
  if( form.sphere.isValid() ){
    form.sphere.activate();
    setViewPosition( form.sphere.x(), form.sphere.y(), form.sphere.z() );

    for( size_t i=0; i<env.size(); ++i ){
      MyGL::Matrix4x4 m;
      m.identity();

      m.mul  ( form.sphere.transform() );
      m.scale( form.sphere.diameter()  );

      if( i<getClass().view.size() ){
        double sphereDiameter = getClass().view[i].sphereDiameter;

        m.scale( getClass().view[i].size[0]/sphereDiameter,
                 getClass().view[i].size[1]/sphereDiameter,
                 getClass().view[i].size[2]/sphereDiameter );

        //if( getClass().view[i].align[2] )
          //m.translate( 0, 0, -getClass().view[i].size[2]/sphereDiameter );
        }
      //m.translate( 0, 0, -0.8 );

      env[i].setTransform( m );
      }
    }

  }

const ProtoObject &GameObject::getClass() const {
  return *myClass;
  }

int GameObject::x() const {
  return m.x;
  }

int GameObject::y() const {
  return m.y;
  }

int GameObject::z() const {
  return m.z;
  }

bool GameObject::isSelected() const {
  return m.isSelected;
  }

void GameObject::select() {
  if( !m.isSelected ){
    m.isSelected = true;
    selection.setVisible( m.isSelected );

    world().player( m.pl ).select( this, 1);
    }
  }

void GameObject::unSelect() {
  if( m.isSelected ){
    m.isSelected  = false;
    selection.setVisible( m.isSelected );

    world().player( m.pl ).select( this, 0);
    }
  }

void GameObject::updateSelection() {
  //m.isSelected  = m.isMouseOwer;
  selection.setVisible( m.isMouseOwer );
  m.isMouseOwer = 0;
  }

void GameObject::setViewDirection(int lx, int ly) {
  m.intentDirX = lx;
  m.intentDirY = ly;
  }

void GameObject::rotate( int delta ) {
  for( size_t i=0; i<view.size(); ++i ){
    double a = view[i].angleZ() + delta;
    m.intentDirX = 10000*cos( M_PI*a/180.0 );
    m.intentDirY = 10000*sin( M_PI*a/180.0 );

    view[i].setRotation( 0, a );
    }
  }

bool GameObject::isMouseOwer() const {
  return m.isMouseOwer;
  }

void GameObject::setMouseOverFlag(bool f) {
  m.isMouseOwer = f;
  selection.setVisible( m.isMouseOwer );
  }

double GameObject::radius() const {
  return m.selectionSize[0]*1.44;
  }

double GameObject::rawRadius() const {
  return m.radius;
  }

MyGL::Matrix4x4 GameObject::_transform() const {
  return view[0].transform();
  }

bool GameObject::isOnMove() const {
  return bclos.isOnMove;
  }

bool GameObject::isRepositionMove() const {
  return bclos.isReposMove;
  }

World &GameObject::world() {
  return wrld;
  }

Game &GameObject::game() {
  return wrld.game;
  }

MyGL::Scene &GameObject::getScene() {
  return scene;
  }

void GameObject::setPlayer(int pl) {
  if( m.pl==pl )
    return;

  wrld.player( m.pl ).delUnit(this);
  m.pl = pl;
  m.teamColor = wrld.player( m.pl ).color();
  wrld.player( m.pl ).addUnit(this);
  }

int GameObject::playerNum() const {
  return m.pl;
  }

int GameObject::team() const {
  return wrld.player(m.pl).team();
  }

bool GameObject::hasHostCtrl() const {
  return wrld.player(m.pl).hasHostControl();
  }

bool GameObject::isMoviable() const {
  return bclos.isMoviable;
  }

bool GameObject::isMineralMove() const {
  return bclos.isMineralMove;
  }

void GameObject::tick( const Terrain &terrain ) {
  double a  = 180.0*atan2(m.intentDirY, m.intentDirX)/M_PI;
  double at = 10;

  for( size_t i=0; i<view.size(); ++i ){
    double az = view[i].angleZ();
    double da = a - az;

    if( fabs(da) > at && !getClass().view[i].randRotate ){
      if( cos( M_PI*da/180.0 ) > cos(M_PI*2*at/180.0 ) )
        view[i].setRotation(0, a); else
      if( sin( M_PI*da/180.0 ) > 0 )
        view[i].setRotation(0, az+at); else
        view[i].setRotation(0, az-at);
      }

    }

  behavior.tick( terrain );
  }
