#include "gameobjectview.h"

#include "prototypesloader.h"
#include "resource.h"
#include "world.h"
#include "game.h"

#include "util/gameserializer.h"

#include <cmath>

GameObjectView::GameObjectView( GameObject &obj,
                                const ProtoObject &p )
  : selection(obj.getScene()),
    scene( obj.getScene() ),
    wrld( obj.world() ),    
    psysEngine( wrld.getParticles() ),
    cls(&p),
    prototypes( obj.prototypes ) {
  m.radius = 0;

  m.x = 0;
  m.y = 0;
  m.z = 0;

  for( int i=0; i<3; ++i )
    m.selectionSize[i] = 0;

  for( int i=0; i<3; ++i )
    m.modelSize[i] = 0;

  m.intentDirX = 0;
  m.intentDirY = -1;


  loadView( obj.game().resources(), wrld.physics, 0 );
  }

GameObjectView::GameObjectView( MyGL::Scene & s,
                                World       & wrld,
                                const ProtoObject &p,
                                const PrototypesLoader & pl )
  : selection(s), scene(s), wrld(wrld),
    psysEngine( wrld.getParticles() ), cls(&p), prototypes(pl) {
  m.radius = 0;

  m.x = 0;
  m.y = 0;
  m.z = 0;

  for( int i=0; i<3; ++i )
    m.selectionSize[i] = 0;

  for( int i=0; i<3; ++i )
    m.modelSize[i] = 0;

  m.intentDirX = 0;
  m.intentDirY = -1;
  }

GameObjectView::GameObjectView( MyGL::Scene & s,
                                World       & wrld,
                                ParticleSystemEngine & psysEngine,
                                const ProtoObject &p,
                                const PrototypesLoader & pl )
  : selection(s), scene(s), wrld(wrld),
    psysEngine(psysEngine), cls(&p), prototypes(pl) {
  m.radius = 0;

  m.x = 0;
  m.y = 0;
  m.z = 0;

  for( int i=0; i<3; ++i )
    m.selectionSize[i] = 0;

  for( int i=0; i<3; ++i )
    m.modelSize[i] = 0;

  m.intentDirX = 0;
  m.intentDirY = -1;
  }

GameObjectView::~GameObjectView() {
  freePhysic();
  }

void GameObjectView::freePhysic(){
  if( physic ){
    physic->free(form.sphere);
    physic->free(form.box);

    physic->free(anim.box);
    physic->free(anim.sphere);
    }
  }

void GameObjectView::loadView( const Resource &r, Physics & p, bool env ) {
  physic = &p;
  view.clear();
  m.radius = 0;

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
        const double *bs = getClass().view[i].boxSize;
        setForm( p.createBox( x(), y(), 0, bs[0], bs[1], bs[2] ));
        } else {
        const double *bs = getClass().view[i].boxSize;
        setForm( p.createAnimatedBox( x(), y(), 0, bs[0], bs[1], bs[2] ));
        }
      }
    }

  const ProtoObject::View & v = prototypes.get("selection.green").view[0];
  setupMaterials( selection, v );
  selection.setModel( r.model("quad/model") );

  setViewSize(1, 1, 1);
  //rotate( 180 );
  }

void GameObjectView::loadView( const Resource & r,
                               const ProtoObject::View &src,
                               bool isEnv ) {
  const Model & model = r.model( src.name+"/model" );

  for( int i=0; i<3; ++i )
    m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];

  MyGL::AbstractGraphicObject * obj = 0;

  if( isEnv ){
    if( !src.isParticle ){
      EnvObject object( scene );
      object.setModel( model );

      env.push_back( object );
      obj = &env.back();
      }else {
      ParticleSystem sys( psysEngine, src );
      particles.push_back( sys );
      }

    } else {
    if( !src.isParticle ){
      MyGL::GraphicObject object( scene );
      object.setModel( model );

      view.push_back( object );
      obj = &view.back();

      if( src.randRotate )
        view.back().setRotation(0, rand()%360 );
      } else {
      ParticleSystem sys( psysEngine, src );
      particles.push_back( sys );
      }
    }

  if( !src.isParticle )
    setupMaterials(*obj, src );

  m.radius = std::max(m.radius, model.bounds().diameter()/2.0 );
  }

void GameObjectView::loadView( const MyGL::Model<> &model ){
  view.clear();

  for( int i=0; i<3; ++i )
    m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];

  MyGL::GraphicObject object( scene );
  object.setModel( model );
  m.radius = model.bounds().radius();

  setupMaterials( object, getClass().view[0] );
  view.push_back( object );
  }

void GameObjectView::loadView( const MyGL::Model<Terrain::WVertex> &model ){
  view.clear();

  for( int i=0; i<3; ++i )
    m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];

  MyGL::GraphicObject object( scene );
  object.setModel( model );
  m.radius = model.bounds().radius();

  setupMaterials( object, getClass().view[0] );
  view.push_back( object );
  }

void GameObjectView::setViewPosition(float x, float y, float z) {
  for( size_t i=0; i<view.size(); ++i ){
    setViewPosition( view[i], getClass().view[i], x, y, z );
    }

  for( size_t i=0; i<particles.size(); ++i ){
    particles[i].setPosition( x, y, z );
    }

  float zland = World::coordCast( wrld.terrain().heightAt( x, y ) );
  selection.setPosition( x, y, std::max(zland, z)+0.01 );

  if( form.sphere.isValid() )
    form.sphere.setPosition(x,y,z);

  if( form.box.isValid() )
    form.box.setPosition(x,y,z);

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
  if( anim.box.isValid() ){
    anim.box.setPosition(x+dx,y+dy,z+dz);
    }
  }

void GameObjectView::setViewPosition( MyGL::GraphicObject& obj,
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

double GameObjectView::viewHeight() const {
  if( view.size() ){
    const int   align = getClass().view[0].align[2];
    double alignSize  = getClass().view[0].alignSize;

    return m.modelSize[2]*view[0].sizeZ()*( 1 - (1-align)*alignSize );
    }

  return 0;
  }

void GameObjectView::setViewSize( float x, float y, float z ) {
  for( size_t i=0; i<view.size(); ++i ){
    setViewSize( view[i], getClass().view[i], x, y, z );
    }

  double ss[3] = { m.radius*x, m.radius*y, z };

  m.selectionSize[2] = ss[2];
  if( view.size() ){
    for( int i=0; i<2; ++i )
      m.selectionSize[i] = ss[i]*getClass().view[0].size[i]/1.44;
    }

  //selection.setVisible( m.isMouseOwer );

  double s = m.selectionSize[0];
  for( int i=0; i<2; ++i )
    s = std::min(s, m.selectionSize[i] );

  selection.setSize( s );
  }

void GameObjectView::setViewSize( MyGL::GraphicObject &obj,
                                  const ProtoObject::View & v,
                                  float x, float y, float z ) {
  const double *s = v.size;
  obj.setSize(s[0]*x, s[1]*y, s[2]*z);
  }

void GameObjectView::setForm(const Physics::Sphere &f) {
  form.sphere = f;
  //animForm = Physics::AnimatedSphere();
  }

void GameObjectView::setForm(const Physics::Box &f) {
  form.box = f;
  //animForm = Physics::AnimatedSphere();
  }

void GameObjectView::setForm(const Physics::AnimatedSphere &f) {
  anim.sphere = f;
  //form = Physics::Sphere();
  }

void GameObjectView::setForm(const Physics::AnimatedBox &f) {
  anim.box = f;
  //form = Physics::Sphere();
  }

void GameObjectView::updatePos() {
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
    updatePosRigid(form.sphere);
    }

  if( form.box.isValid() ){
    updatePosRigid(form.box);
    }
  }

void GameObjectView::setSelectionVisible(bool v) {
  selection.setVisible(v);
  }

void GameObjectView::setVisible(bool v) {
  for( size_t i=0; i<view.size(); ++i )
    view[i].setVisible(v);
  }

void GameObjectView::rotate( int delta ) {
  for( size_t i=0; i<view.size(); ++i ){
    double a = view[i].angleZ() + delta;
    m.intentDirX = 10000*cos( M_PI*a/180.0 );
    m.intentDirY = 10000*sin( M_PI*a/180.0 );

    view[i].setRotation( 0, a );
    }
  }

void GameObjectView::setRotation( int a ) {
  for( size_t i=0; i<view.size(); ++i ){
    m.intentDirX = 10000*cos( M_PI*a/180.0 );
    m.intentDirY = 10000*sin( M_PI*a/180.0 );

    view[i].setRotation( 0, a );
    }
  }

void GameObjectView::setViewDirection(int lx, int ly) {
  if( lx==0 && ly==0 )
    return;

  m.intentDirX = lx;
  m.intentDirY = ly;
  }

void GameObjectView::viewDirection(int &x, int &y) const {
  x = m.intentDirX;
  y = m.intentDirY;
  }

double GameObjectView::rAngle() const {
  if( view.size() )
    return view[0].angleZ();
  return 0;
  }

void GameObjectView::tick() {
  double a  = 180.0*atan2( m.intentDirY, m.intentDirX )/M_PI;
  double at = cls->rotateSpeed;

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
}

template< class Rigid >
void GameObjectView::updatePosRigid( Rigid &rigid ){
  // rigid.activate();
  setViewPosition( rigid.x(), rigid.y(), rigid.z() );

  for( size_t i=0; i<env.size(); ++i ){
    updatePosRigid(rigid, i);
    }
  }

void GameObjectView::updatePosRigid( Physics::Sphere &rigid, size_t i ) {
  MyGL::Matrix4x4 m;
  m.identity();

  m.mul  ( rigid.transform() );
  m.scale( rigid.diameter()  );

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

void GameObjectView::updatePosRigid( Physics::Box &rigid, size_t i ) {
  MyGL::Matrix4x4 m;
  m.identity();

  m.mul  ( rigid.transform() );
  const double * bs = getClass().view[i].boxSize;
  m.scale( bs[0], bs[1], bs[2] );

  if( i<getClass().view.size() ){
    const double * lbs = getClass().view[i].boxSize;

    m.scale( getClass().view[i].size[0]/lbs[0],
             getClass().view[i].size[1]/lbs[1],
             getClass().view[i].size[2]/lbs[2] );

    //if( getClass().view[i].align[2] )
      //m.translate( 0, 0, -getClass().view[i].size[2]/sphereDiameter );
    }
  //m.translate( 0, 0, -0.8 );

  env[i].setTransform( m );
  }

int GameObjectView::x() const {
  return m.x;
  }

int GameObjectView::y() const {
  return m.y;
  }

void GameObjectView::setPosition(int x, int y) {
  m.x = x;
  m.y = y;
  }

double GameObjectView::radius() const {
  return m.selectionSize[0]*1.44;
  }

double GameObjectView::rawRadius() const {
  return m.radius;
  }

Physics *GameObjectView::physicEngine() const {
  return physic;
  }

const ProtoObject &GameObjectView::getClass() const {
  return *cls;
  }

void GameObjectView::setupMaterials( MyGL::AbstractGraphicObject &obj,
                                     const ProtoObject::View &src ) {
  wrld.game.setupMaterials( obj, src, teamColor );
  }

void GameObjectView::serialize( GameSerializer &s ) {
  bool v = selection.isVisible();
  s + v;
  selection.setVisible(v);

  unsigned vsize = view.size();
  s + vsize;

  vsize = std::min( vsize, view.size() );

  const int mulI = 10000;

  for( unsigned i=0; i<vsize; ++i ){
    MyGL::GraphicObject & g = view[i];
    int x = World::coordCastD(g.x()),
        y = World::coordCastD(g.y()),
        z = World::coordCastD(g.z()),

        az = g.angleZ()*mulI,
        ax = g.angleX()*mulI;

    s + x +
        y +
        z +
        az +
        ax +
        m.intentDirX +
        m.intentDirY;

    g.setPosition( World::coordCast(x),
                   World::coordCast(y),
                   World::coordCast(z));

    g.setRotation( ax/double(mulI), az/double(mulI) );
    }
  }
