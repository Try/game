#include "gameobjectview.h"

#include "prototypesloader.h"
#include "resource.h"
#include "world.h"
#include "game.h"

#include "util/gameserializer.h"
#include "graphics/smallobjectsview.h"

#include <cmath>

GameObjectView::GameObjectView( GameObject &obj,
                                const ProtoObject &p )
  : scene( obj.getScene() ),
    wrld( obj.world() ),    
    psysEngine( wrld.getParticles() ),
    cls(&p),
    prototypes( obj.prototypes ) {
  init();
  loadView( obj.game().resources(), wrld.physics, 0 );
  }

GameObjectView::GameObjectView( MyGL::Scene & s,
                                World       & wrld,
                                const ProtoObject &p,
                                const PrototypesLoader & pl )
  : scene(s), wrld(wrld),
    psysEngine( wrld.getParticles() ), cls(&p), prototypes(pl) {
  init();
  }

GameObjectView::GameObjectView( MyGL::Scene & s,
                                World       & wrld,
                                ParticleSystemEngine & psysEngine,
                                const ProtoObject &p,
                                const PrototypesLoader & pl )
  : scene(s), wrld(wrld),
    psysEngine(psysEngine), cls(&p), prototypes(pl) {
  init();
  }

void GameObjectView::init() {
  m.radius = 0;

  m.x = 0;
  m.y = 0;
  m.z = 0;

  for( int i=0; i<3; ++i ){
    m.selectionSize[i] = 0;
    m.modelSize[i] = 0;
    m.rndSize[i] = 1;
    }

  m.intentDirX = 0;
  m.intentDirY = -1;

  for( int i=0; i<selectModelsCount; ++i ){
    selection[i].reset( new MyGL::GraphicObject(scene) );
    selection[i]->setVisible(false);
    htime[i] = 0;
    }
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

  float ksize = rand()/float(RAND_MAX);//getClass().sizeBounds;
  double *sz = m.rndSize;

  for( int i=0; i<3; ++i ){
    double diff = getClass().sizeBounds.max[i]-getClass().sizeBounds.min[i];
    sz[i] = getClass().sizeBounds.min[i]+diff*ksize;
    }

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

  bool res = false;

  for( size_t i=0; i<getClass().behaviors.size(); ++i )
    if( getClass().behaviors[i]=="resource" )
      res = true;

  if( !getClass().data.isBackground || res ){
    { const ProtoObject::View & v = prototypes.get("selection.green").view[0];
      setupMaterials( *selection[0], v );
      }

    { const ProtoObject::View & v = prototypes.get("selection.over").view[0];
      setupMaterials( *selection[1], v );
      }

    { const ProtoObject::View & v = prototypes.get("selection.moveTo").view[0];
      setupMaterials( *selection[2], v );
      }

    { const ProtoObject::View & v = prototypes.get("selection.moveTo").view[0];
      setupMaterials( *selection[3], v );
      }

    { const ProtoObject::View & v = prototypes.get("selection.atkTo").view[0];
      setupMaterials( *selection[4], v );
      }

    for( int i=0; i<selectModelsCount; ++i )
      selection[i]->setModel( r.model("quad/model") );
    }

  setViewSize(1, 1, 1);
  //rotate( 180 );
  }

void GameObjectView::loadView( const Resource & r,
                               const ProtoObject::View &src,
                               bool isEnv ) {
  const Model & model = r.model( src.name+"/model" );

  if( view.size()==0 ){
    for( int i=0; i<3; ++i )
      m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];
    }

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
      if( model.size() > 64*3 || !getClass().data.isBackground ){
        MyGL::GraphicObject object( scene );
        object.setModel( model );

        view.push_back( object );
        if( src.randRotate ){
          view.back().setRotation(0, rand()%360 );
          }

        obj = &view.back();
        } else {
        SmallGraphicsObject* object = new SmallGraphicsObject( scene,
                                                               wrld.game,
                                                               wrld.terrain(),
                                                               src );
        object->setModel( model, src.name+"/model" );

        smallViews.push_back( std::unique_ptr<SmallGraphicsObject>(object) );
        if( src.randRotate ){
          smallViews.back()->setRotation(0, rand()%360 );
          }
        }

      //view.back().setSize( sz[0], sz[1], sz[2] );
      } else {
      ParticleSystem sys( psysEngine, src );
      particles.push_back( sys );
      }
    }

  if( obj ){
    setupMaterials(*obj, src );
    }

  m.radius = std::max(m.radius, model.bounds().diameter()/2.0 );
  }

void GameObjectView::loadView( const Model &model,
                               const ProtoObject::View & pview ){
  view.clear();

  for( int i=0; i<3; ++i )
    m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];

  MyGL::GraphicObject object( scene );
  object.setModel( model );
  m.radius = model.bounds().radius();

  setupMaterials( object, pview );
  view.push_back( object );
  }

void GameObjectView::loadView(const MyGL::Model<WaterVertex> &model ){
  view.clear();

  for( int i=0; i<3; ++i )
    m.modelSize[i] = model.bounds().max[i]-model.bounds().min[i];

  MyGL::GraphicObject object( scene );
  object.setModel( model );
  m.radius = model.bounds().radius();

  setupMaterials( object, getClass().view[0] );
  view.push_back( object );
  }

void GameObjectView::setViewPosition( float x, float y, float z ) {
  setViewPosition(x,y,z,1);
  }

void GameObjectView::setViewPosition( float x, float y, float z,
                                      float interp ) {
  for( size_t i=0; i<env.size(); ++i ){
    //setViewPosition( env[i], getClass().view[i], x, y, z );
    }

  for( size_t i=0; i<view.size(); ++i ){
    setViewPosition( view[i], getClass().view[i], x, y, z, interp );
    }

  for( size_t i=0; i<smallViews.size(); ++i ){
    setViewPosition( *smallViews[i], smallViews[i]->view, x, y, z, interp );
    }

  for( size_t i=0; i<particles.size(); ++i ){
    particles[i].setPosition( x, y, z );
    }

  float wx = World::coordCastD(x)/Terrain::quadSizef,
        wy = World::coordCastD(y)/Terrain::quadSizef;
  //float zland = World::coordCast( std::max( wrld.terrain().heightAt(wx,wy),
  //                                          wrld.terrain().atF(wx,wy) ) );
  float zland = World::coordCast( wrld.terrain().heightAt(wx,wy) );

  for( int i=0; i<selectModelsCount; ++i )
    selection[i]->setPosition( x, y, zland+0.01 );

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

template< class Object >
void GameObjectView::setViewPosition( Object& obj,
                                      const ProtoObject::View & v,
                                      float x,
                                      float y,
                                      float z,
                                      float interp ) {
  float dx = 0, dy = 0, dz = 0;
  double modelSize[3] = { obj.bounds().max[0] - obj.bounds().min[0],
                          obj.bounds().max[1] - obj.bounds().min[1],
                          obj.bounds().max[2] - obj.bounds().min[2] };

  const int * align = v.align;
  double alignSize  = v.alignSize;

  dx = modelSize[0]*obj.sizeX()*align[0]*alignSize;
  dy = modelSize[1]*obj.sizeY()*align[1]*alignSize;
  dz = modelSize[2]*obj.sizeZ()*align[2]*alignSize;

  float lx = obj.x() + (x+dx - obj.x())*interp;
  float ly = obj.y() + (y+dy - obj.y())*interp;
  float lz = obj.z() + (z+dz - obj.z())*interp;

  if( lx!=obj.x() || ly!=obj.y() || lz!=obj.z() )
    obj.setPosition( lx,ly,lz );
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

  for( size_t i=0; i<smallViews.size(); ++i ){
    setViewSize( *smallViews[i], smallViews[i]->view, x, y, z );
    }

  double ss[3] = { m.radius*x, m.radius*y, z };

  m.selectionSize[2] = ss[2];
  if( getClass().view.size() ){
    for( int i=0; i<2; ++i )
      m.selectionSize[i] = ss[i]*getClass().view[0].size[i]/1.44;
    }

  //selection.setVisible( m.isMouseOwer );

  double s = m.selectionSize[0];
  for( int i=0; i<2; ++i )
    s = std::min(s, m.selectionSize[i] );

  selection[0]->setSize( s );

  for( int i=1; i<selectModelsCount; ++i )
    selection[i]->setSize( s*1.05 );
  }

template< class Object >
void GameObjectView::setViewSize( Object &obj,
                                  const ProtoObject::View & v,
                                  float x, float y, float z ) {
  const double *s = v.size;  

  obj.setSize( s[0]*x*m.rndSize[0],
               s[1]*y*m.rndSize[1],
               s[2]*z*m.rndSize[2] );
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

void GameObjectView::setSelectionVisible( bool v, Selection s ) {
  selection[int(s)]->setVisible(v);
  }

void GameObjectView::higlight(int time, GameObjectView::Selection s) {
  htime[int(s)] = time;
  selection[int(s)]->setVisible(true);
  }

void GameObjectView::setVisible_perf(bool v) {
  for( size_t i=0; i<view.size(); ++i )
    view[i].setVisible(v);
  }

void GameObjectView::setVisible(bool v) {
  for( size_t i=0; i<view.size(); ++i )
    view[i].setVisible(v);

  for( size_t i=0; i<smallViews.size(); ++i )
    smallViews[i]->setVisible(v);
  }

void GameObjectView::rotate( int delta ) {
  for( size_t i=0; i<view.size(); ++i ){
    double a = view[i].angleZ() + delta;
    m.intentDirX = 10000*cos( M_PI*a/180.0 );
    m.intentDirY = 10000*sin( M_PI*a/180.0 );

    view[i].setRotation( 0, a );
    }

  for( size_t i=0; i<smallViews.size(); ++i ){
    double a = smallViews[i]->angleZ() + delta;
    m.intentDirX = 10000*cos( M_PI*a/180.0 );
    m.intentDirY = 10000*sin( M_PI*a/180.0 );

    smallViews[i]->setRotation( 0, a );
    }
  }

void GameObjectView::setRotation( int a ) {
  m.intentDirX = 10000*cos( M_PI*a/180.0 );
  m.intentDirY = 10000*sin( M_PI*a/180.0 );

  for( size_t i=0; i<view.size(); ++i ){
    view[i].setRotation( 0, a );
    }

  for( size_t i=0; i<smallViews.size(); ++i ){
    smallViews[i]->setRotation( 0, a );
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
  for( int i=1; i<selectModelsCount; ++i ){
    selection[i]->setRotation( selection[i]->angleX(),
                               selection[i]->angleZ()+1 );
    }

  for( int i=2; i<selectModelsCount; ++i ){
    if( htime[i] ){
      htime[i]--;
      if( htime[i]==0 )
        selection[i]->setVisible(false);
      }
    }

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
      } else {
      view[i].setRotation(0, a);
      }
    }

  for( size_t i=0; i<smallViews.size(); ++i ){
    smallViews[i]->setRotation( 0, a );
    double az = smallViews[i]->angleZ();
    double da = a - az;

    if( fabs(da) > at && !smallViews[i]->view.randRotate ){
      if( cos( M_PI*da/180.0 ) > cos(M_PI*2*at/180.0 ) )
        smallViews[i]->setRotation(0, a); else
      if( sin( M_PI*da/180.0 ) > 0 )
        smallViews[i]->setRotation(0, az+at); else
        smallViews[i]->setRotation(0, az-at);
      } else {
      smallViews[i]->setRotation(0, a);
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

MyGL::Matrix4x4 GameObjectView::_transform() const {
  if( view.size()==0 ){
    if( smallViews.size() )
      return smallViews[0]->transform(); else
      return MyGL::Matrix4x4();
    }

  return view[0].transform();
  }

void GameObjectView::setupMaterials( MyGL::AbstractGraphicObject &obj,
                                     const ProtoObject::View &src ) {
  wrld.game.setupMaterials( obj, src, teamColor );
  }

void GameObjectView::serialize( GameSerializer &s ) {
  if( s.version()<4 ){
    bool v = false;//selection.isVisible();
    s + v;
    //selection.setVisible(v);
    }

  unsigned vsize = view.size();
  s + vsize;

  //vsize = view.size();//std::min( vsize, view.size() );

  if( s.version()>=2 ){
    int rndS[3];
    for( int i=0; i<3; ++i )
      rndS[i] = m.rndSize[i]*10000;

    for( int i=0; i<3; ++i )
      s + rndS[i];

    for( int i=0; i<3; ++i )
      m.rndSize[i] = rndS[i]/10000.0;
    } else {
    for( int i=0; i<3; ++i )
      m.rndSize[i] = 1;
    }

  if( s.version()>=3 ){
    unsigned ssize = smallViews.size();
    s + ssize;

    for( unsigned i=0; i<vsize; ++i ){
      bool st = i<view.size();
      serialize( s, st ? &view[i]:0, st );
      }

    for( unsigned i=0; i<ssize; ++i ){
      bool st = i<smallViews.size();
      serialize( s, st ? smallViews[i].get():0, st );
      }

    } else {
    for( unsigned i=0; i<view.size(); ++i ){
      bool st = i<view.size();
      serialize( s, st ? &view[i]:0, st );
      }

    if( vsize>view.size() ){
      unsigned sc = std::min( smallViews.size(), vsize-view.size() );
      for( unsigned i=0; i<sc; ++i ){
        bool st = i<smallViews.size();
        serialize( s, st ? smallViews[i].get():0, st );
        }
      }
    }
  }

template< class Obj >
void GameObjectView::serialize( GameSerializer &s, Obj *g,
                                bool store ) {
  const int mulI = 10000;

  int x = 0,
      y = 0,
      z = 0,

      az = 0,
      ax = 0;

  int sz[3] = {};
  if( !s.isReader() ){
    x = World::coordCastD(g->x());
    y = World::coordCastD(g->y());
    z = World::coordCastD(g->z());

    az = g->angleZ()*mulI;
    ax = g->angleX()*mulI;

    sz[0] = g->sizeX()*10000;
    sz[1] = g->sizeY()*10000;
    sz[2] = g->sizeZ()*10000;
    }

  s + x +
      y +
      z +
      az +
      ax +
      m.intentDirX +
      m.intentDirY;

  if( s.version()>=2 ){
    for( int i=0; i<3; ++i )
      s + sz[i];
    }

  if( store ){
    if( s.version()<5 ){
      x*=4;
      y*=4;
      z*=4;
      }

    g->setPosition( World::coordCast(x),
                    World::coordCast(y),
                    World::coordCast(z));

    g->setRotation( ax/double(mulI), az/double(mulI) );

    if( s.version()>=2 ){
      //if( getClass().name!="pikeman" )
        g->setSize( sz[0]/10000.0, sz[1]/10000.0, sz[2]/10000.0 );
      }
    }
  }
