#include "gameobject.h"

#include <MyGL/GraphicObject>

#include "resource.h"

#include "game.h"

#include "game/world.h"
#include "prototypesloader.h"

#include "algo/algo.h"
#include <cmath>
#include "game/player.h"

#include "bullet.h"

#include "util/gameserializer.h"

GameObject::GameObject( MyGL::Scene & s,
                        World &w,
                        const ProtoObject &p,
                        const PrototypesLoader &pl )
  : scene(s),
    wrld(w),
    prototypes(pl),
    view(s, w, p, pl),
    myClass(&p){
  m.x = 0;
  m.y = 0;
  m.z = 0;

  m.isVisible_perf = true;

  bclos.colisionDisp[0] = 0;
  bclos.colisionDisp[1] = 0;

  m.hp = p.data.maxHp;

  m.pl = 0;
  wrld.player( m.pl ).addUnit(this);
  view.teamColor = wrld.player( m.pl ).color();

  view.setViewDirection(1, 0);

  m.isSelected = false;

  setMouseOverFlag(0);

  behavior.bind( *this, bclos );

  for( auto i=p.behaviors.begin(); i!=p.behaviors.end(); ++i ){
    behavior.add( *i );
    }
  }

GameObject::~GameObject() {
  //if( behavior.size() )
  wrld.player( m.pl ).delUnit(this);
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

void GameObject::setupMaterials( MyGL::AbstractGraphicObject &obj,
                                 const ProtoObject::View &src ) {
  game().setupMaterials( obj, src, teamColor() );
  }

void GameObject::loadView( const Resource &r, Physics & p, bool env ) {
  view.loadView(r, p, env );

  if( env ){
    if( behavior.size() )
      wrld.player( m.pl ).delUnit(this);

    behavior.clear();
    }

  rotate( 180 );
  }

void GameObject::loadView( Resource & r,
                           const ProtoObject::View &src,
                           bool isEnv ) {
  view.loadView( r, src, isEnv );
  }

void GameObject::loadView( const Model &model,
                           const ProtoObject::View & pview ){
  view.loadView( model, pview );
  }

void GameObject::loadView(const MyGL::Model<WaterVertex> &model ){
  view.loadView( model );
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

void GameObject::setHP( int hp ) {
  m.hp = hp;
  }

int GameObject::hp() const {
  return m.hp;
  }

void GameObject::setViewPosition(float x, float y, float z, float s) {
  view.setViewPosition(x,y,z, s);
  }

double GameObject::viewHeight() const {
  return view.viewHeight();
  }

const MyGL::Color& GameObject::teamColor() const {
  return view.teamColor;
  }

void GameObject::setTeamColor(const MyGL::Color &cl) {
  view.teamColor = cl;
  }

void GameObject::setPosition(int x, int y, int z) {
  setViewPosition( World::coordCast(x),
                   World::coordCast(y),
                   World::coordCast(z),
                   1 );
  m.x = x;
  m.y = y;
  m.z = z;//world.terrain().heightAt(x, y);
  view.setPosition(x,y);

  behavior.message( Behavior::onPositionChange, x, y );
  }

void GameObject::setPositionSmooth(int x, int y, int z) {
  setViewPosition( World::coordCast(x),
                   World::coordCast(y),
                   World::coordCast(z),
                   0.3 );
  m.x = x;
  m.y = y;
  m.z = z;//world.terrain().heightAt(x, y);
  view.setPosition(x,y);

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
  view.setViewSize(x,y,z);
  }

void GameObject::updatePos() {
  view.updatePos();
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
    //selection.setVisible( m.isSelected );
    view.setSelectionVisible( m.isSelected, GameObjectView::selStd );

    world().player( m.pl ).select( this, 1);
    }
  }

void GameObject::unSelect() {
  if( m.isSelected ){
    m.isSelected  = false;
    view.setSelectionVisible( m.isSelected, GameObjectView::selStd );

    world().player( m.pl ).select( this, 0);
    }
  }

void GameObject::updateSelection() {
  view.setSelectionVisible( m.isMouseOwer && (!getClass().data.isBackground),
                            GameObjectView::selOver );
  m.isMouseOwer = 0;
  }

void GameObject::setViewDirection(int lx, int ly) {
  view.setViewDirection(lx, ly);
  }

void GameObject::viewDirection(int &x, int &y) {
  view.viewDirection(x,y);
  }

void GameObject::rotate( int delta ) {
  view.rotate( delta );
  }

double GameObject::rAngle() const {
  return view.rAngle()*M_PI/180.0;
  }

bool GameObject::isMouseOwer() const {
  return m.isMouseOwer;
  }

void GameObject::setMouseOverFlag(bool f) {
  m.isMouseOwer = f;
//  selection.setVisible( m.isMouseOwer );
  view.setSelectionVisible( m.isMouseOwer, GameObjectView::selOver );
  }

double GameObject::radius() const {
  return view.radius();
  }

double GameObject::rawRadius() const {
  return view.rawRadius();
  }

MyGL::Matrix4x4 GameObject::_transform() const {
  return view._transform();
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
  view.teamColor = wrld.player( m.pl ).color();

  wrld.player( m.pl ).addUnit(this);
  }

int GameObject::playerNum() const {
  return m.pl;
  }

Player &GameObject::player() {
  return wrld.player( playerNum() );
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
  setViewPosition( World::coordCast(m.x),
                   World::coordCast(m.y),
                   World::coordCast(m.z),
                   0.3 );

  view.tick();
  behavior.tick( terrain );

  for( size_t i=0; i<bullets.size(); ++i )
    bullets[i]->tick();

  for( size_t i=0; i<bullets.size(); )
    if( bullets[i]->isFinished ){
      setHP( hp() - bullets[i]->absDmg );
      std::swap( bullets[i], bullets.back() );
      bullets.pop_back();
      } else {
      ++i;
      }
  }

std::shared_ptr<Bullet> GameObject::reciveBulldet( const std::string &v ){
  assert( view.physicEngine() );

  Bullet *b = new Bullet( scene, wrld,
                          prototypes.get(v),
                          prototypes );

  b->view.loadView( game().resources(), wrld.physics, 0 );
  b->tgX = x();
  b->tgY = y();

  bullets.push_back( std::shared_ptr<Bullet>(b) );
  return bullets.back();
  }

void GameObject::serialize( GameSerializer &s ) {
  int x = m.x,
      y = m.y,
      z = m.z,
      pl = m.pl;

  s + m.hp +
      pl +
      x  +
      y  +
      z;

  if( s.version()<5 ){
    x*=4;
    y*=4;
    z*=4;
    }

  if( s.isReader() ){
    setPosition( x, y, z );
    setPlayer( pl );
    }

  view.serialize(s);
  }

void GameObject::setColisionDisp(int dx, int dy) {
  bclos.colisionDisp[0] = dx;
  bclos.colisionDisp[1] = dy;
  }

void GameObject::incColisionDisp(int dx, int dy) {
  bclos.colisionDisp[0] += dx;
  bclos.colisionDisp[1] += dy;
  }

void GameObject::higlight(int time, GameObjectView::Selection type) {
  view.higlight(time, type);
  }

void GameObject::setVisible_perf(bool v) {
  view.setVisible_perf(v);
  m.isVisible_perf = v;
  }

bool GameObject::isVisible_perf() const {
  return m.isVisible_perf;
  }
