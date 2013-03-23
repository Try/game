#include "physics.h"

#ifndef NO_PHYSIC
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#endif

#include <iostream>

#include "game/world.h"

#include <memory>
#include <unordered_set>

struct Physics::Data{
#ifndef NO_PHYSIC
  std::unique_ptr<btBroadphaseInterface>               broadphase;
  std::unique_ptr<btDefaultCollisionConfiguration>     collisionConfiguration;
  std::unique_ptr<btCollisionDispatcher>               dispatcher;
  std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

  std::unique_ptr<Physics::TerrainBody>                terrain;
  std::unique_ptr<btDiscreteDynamicsWorld>             dynamicsWorld;

  Mutex  physicMutex;

  static btVector3 localInertion( float mass,
                                  const btSphereShape & s ){
    btVector3 out;
    s.calculateLocalInertia( mass, out );

    return out;
    }

  static btVector3 localInertion( float mass,
                                  const btBoxShape & s ){
    btVector3 out;
    s.calculateLocalInertia( mass, out );

    return out;
    }

  int animCount, rigidCount;

  enum PoolException {
    OutOfRigidPool,
    OutOfAnimatedPool
    };

  static void outOfPoolReport( PoolException p ){
    if( p==OutOfAnimatedPool )
      std::cout << "physics: out of animated pool" << std::endl; else
    if( p==OutOfRigidPool )
      std::cout << "physics: out of rigid pool" << std::endl; else
      std::cout << "physics: too many zerglings!!!" << std::endl;
    }
#endif

  };

struct Physics::RigidBody{
#ifndef NO_PHYSIC
  RigidBody( float x, float y, float z, float d, float mass )
    : mstate(
        btTransform( btQuaternion(0,0,0,1),
                     btVector3(x,y,z) )
        ),
      sphere(d/2.0),
      box( btVector3() ),
      body( btRigidBody::btRigidBodyConstructionInfo(
              mass,
              &mstate,
              &sphere,
              Data::localInertion( mass, sphere )
              )
            ) {
    init();
    }

  RigidBody( float  x, float  y, float  z,
             float sx, float sy, float sz, float mass )
    : mstate(
        btTransform( btQuaternion(0,0,0,1),
                     btVector3(x,y,z) )
        ),
      sphere(0),
      box( btVector3(sx/2, sy/2, sz/2) ),
      body( btRigidBody::btRigidBodyConstructionInfo(
              mass,
              &mstate,
              &box,
              Data::localInertion( mass, box )
              )
            ) {
    init();
    }

  void init(){
    //body.setActivationState( DISABLE_DEACTIVATION );
    body.setDeactivationTime(8000);
    body.setFriction(0.5);
    body.setDamping( 0.7, 0.7 );
    //body.setRestitution(0.5);
    }

  btDefaultMotionState mstate;

  btSphereShape        sphere;
  btBoxShape           box;

  btRigidBody          body;
#endif
  };

struct Physics::AnimatedBody:public Physics::RigidBody {
#ifndef NO_PHYSIC
  AnimatedBody( float x, float y, float z, float d ): RigidBody(x,y,z,d,0){}
  AnimatedBody( float  x, float  y, float  z,
                float sx, float sy, float sz ): RigidBody(x,y,z,sx,sy,sz,0){}
#endif
  };

struct Physics::TerrainBody{
#ifndef NO_PHYSIC
  TerrainBody( const Terrain &t,
               const std::vector<float> & data,
               float min,
               float max )
    : mstate(
        btTransform( btQuaternion(0,0,0, 1),
                     btVector3( World::coordCastP( (t.width() -1)*Terrain::quadSize/2.0),
                                World::coordCastP( (t.height()-1)*Terrain::quadSize/2.0),
                                0.5f*(min+max) ) )
        ),
      heightData( data ),
      hmap( t.width(),
            t.height(),
            heightData.data(),
            1,
            min,
            max,
            2,
            PHY_FLOAT,
            false ),
      body( btRigidBody::btRigidBodyConstructionInfo(
              0,
              &mstate,
              &hmap,
              btVector3(0,0,0)
              )
            ) {
    float s = World::coordCast( Terrain::quadSize );
    hmap.setLocalScaling( btVector3(s,s,1) );
    }

  btDefaultMotionState        mstate;
  std::vector<float>          heightData;
  btHeightfieldTerrainShape   hmap;
  btRigidBody                 body;
#endif
  };


Physics::Physics(int /*tw*/, int /*th*/) {
  data = new Data();

#ifndef NO_PHYSIC
  Lock lock(data->physicMutex);
  (void)lock;

  data->broadphase.reset( new btDbvtBroadphase() );

  data->collisionConfiguration.reset( new btDefaultCollisionConfiguration() );
  data->dispatcher.reset( new btCollisionDispatcher( data->collisionConfiguration.get() ) );

  data->solver.reset( new btSequentialImpulseConstraintSolver );

  data->dynamicsWorld.reset( new btDiscreteDynamicsWorld(
                              data->dispatcher.get(),
                              data->broadphase.get(),
                              data->solver.get(),
                              data->collisionConfiguration.get() ) );

  data->dynamicsWorld->setGravity( btVector3( 0, 0, -5 ) );
#endif
  }

Physics::~Physics() {
#ifndef NO_PHYSIC
  { Lock lock(data->physicMutex);
    (void)lock;

    if( data->terrain )
      data->dynamicsWorld->removeRigidBody( &data->terrain->body );
    }
#endif

  delete data;
  }

void Physics::tick() {
#ifndef NO_PHYSIC
  Lock lock(data->physicMutex);
  (void)lock;

  data->dynamicsWorld->stepSimulation( 1/60.f, 5 );
#endif
  }

void Physics::setTerrain( const Terrain &t ) {
#ifndef NO_PHYSIC
  std::vector<float> heightMap( t.width()*t.height() );

  float min = World::coordCast( t.at(0,0) ), max = min;

  for( int i=0; i<t.width(); ++i )
    for( int r=0; r<t.height(); ++r ){
      float h = World::coordCast( t.heightAt(i,r) );
      heightMap[ i+r*t.width() ] = h;

      min = std::min(min, h);
      max = std::max(max, h);
      }

  Lock lock(data->physicMutex);
  (void)lock;

  if( data->terrain )
    data->dynamicsWorld->removeRigidBody( &data->terrain->body );

  TerrainBody *
  terrain = new TerrainBody ( t,
                              heightMap,
                              min,
                              max
                              );
  data->terrain.reset( terrain );

  data->dynamicsWorld->addRigidBody( &data->terrain->body );
  //btDiscreteDynamicsWorld *w = data->dynamicsWorld.get();
#endif
  //w->;
  }


Physics::Sphere Physics::createSphere( float x, float y, float z, float d ) {
  Physics::Sphere s;

#ifndef NO_PHYSIC
  Lock lock(data->physicMutex);
  (void)lock;

  s.data = new RigidBody( x, y, z, d, 0.1 );
  data->dynamicsWorld->addRigidBody( &s.data->body );
#endif
  return s;
  }

void Physics::free( Physics::Rigid &s) {
#ifndef NO_PHYSIC
  if( s.data ){
    Lock lock(data->physicMutex);
    (void)lock;

    data->dynamicsWorld->removeRigidBody( &s.data->body );
    --data->rigidCount;
    s.data = 0;
    }
#endif
  }

Physics::Box Physics::createBox(float x, float y, float z,
                                float sx, float sy, float sz) {
  Physics::Box s;

#ifndef NO_PHYSIC
  Lock lock(data->physicMutex);
  (void)lock;

  s.data = new RigidBody( x, y, z, sx, sy, sz, 0.1 );
  s.engine = data;
  data->dynamicsWorld->addRigidBody( &s.data->body );
#endif

  return s;
  }

Physics::AnimatedSphere Physics::createAnimatedSphere( float x, float y,
                                                       float z, float d ){
  Physics::AnimatedSphere s;

#ifndef NO_PHYSIC
  Lock lock(data->physicMutex);
  (void)lock;

  s.data = new AnimatedBody( x, y, z, d );
  s.engine = data;
  data->dynamicsWorld->addRigidBody( &s.data->body );
#endif

  return s;
  }

Physics::AnimatedBox Physics::createAnimatedBox( float  x, float  y, float  z,
                                                 float sx, float sy, float sz ){
  Physics::AnimatedBox s;

#ifndef NO_PHYSIC
  Lock lock(data->physicMutex);
  (void)lock;

  s.data = new AnimatedBody( x, y, z, sx, sy, sz );
  s.engine = data;
  data->dynamicsWorld->addRigidBody( &s.data->body );
#endif

  return s;
  }

void Physics::free(Physics::AnimatedSphere &s) {
#ifndef NO_PHYSIC
  if( s.data ){
    Lock lock(data->physicMutex);
    (void)lock;

    data->dynamicsWorld->removeRigidBody( &s.data->body );
    --data->animCount;
    s.data = 0;
    }
#endif
  }

void Physics::free(Physics::AnimatedBox &s) {
#ifndef NO_PHYSIC
  if( s.data ){
    Lock lock(data->physicMutex);
    (void)lock;

    data->dynamicsWorld->removeRigidBody( &s.data->body );
    --data->animCount;
    s.data = 0;
    }
#endif
  }

void Physics::beginUpdate() {
#ifndef NO_PHYSIC
  data->physicMutex.lock();
#endif
  }

void Physics::endUpdate() {
#ifndef NO_PHYSIC
  data->physicMutex.unlock();
#endif
  }

Physics::Rigid::Rigid() {
  data = 0;
  engine = 0;

  static const double nullf[16] = {};
  matrix = Tempest::Matrix4x4(nullf);
  }

float Physics::Rigid::x() const {
#ifndef NO_PHYSIC
  Lock lock(engine->physicMutex);
  (void)lock;

  btTransform trans;
  data->body.getMotionState()->getWorldTransform(trans);

  return trans.getOrigin().x();
#else
  return 0;
#endif
  }

float Physics::Rigid::y() const {
#ifndef NO_PHYSIC
  Lock lock(engine->physicMutex);
  (void)lock;

  btTransform trans;
  data->body.getMotionState()->getWorldTransform(trans);

  return trans.getOrigin().y();
#else
  return 0;
#endif
  }

float Physics::Rigid::z() const {
#ifndef NO_PHYSIC
  Lock lock(engine->physicMutex);
  (void)lock;

  btTransform trans;
  data->body.getMotionState()->getWorldTransform(trans);

  return trans.getOrigin().z();
#else
  return 0;
#endif
  }

Tempest::Matrix4x4 Physics::Rigid::transform() {
  return matrix;
  }

float Physics::Rigid::diameter() const {
#ifndef NO_PHYSIC
  return data->sphere.getRadius()*2;
#else
  return 0;
#endif
  }

bool Physics::Rigid::isValid() const {
  return data!=0;
  }

void Physics::Rigid::activate() {
#ifndef NO_PHYSIC
  if( Lock(engine->physicMutex) )
    data->body.activate(true);
#endif
  }

bool Physics::Rigid::isActive() {
#ifndef NO_PHYSIC
  if( Lock(engine->physicMutex) )
    return data->body.isActive();
#endif
  return 0;
  }

void Physics::Rigid::setPosition(float x, float y, float z) {
#ifndef NO_PHYSIC
  btTransform tr;
  data->body.getMotionState()->getWorldTransform(tr);
  tr.setOrigin( btVector3(x,y,z) );

  data->mstate.setWorldTransform( tr );
  data->body.setCenterOfMassTransform( tr );
#endif
  }

void Physics::Rigid::setAngle(float rx, float ry) { 
#ifndef NO_PHYSIC
  rx = rx*M_PI/180.0;
  ry = ry*M_PI/180.0;

  btTransform tr;
  data->body.getMotionState()->getWorldTransform(tr);
  tr.setRotation( btQuaternion(0, rx, ry));

  data->mstate.setWorldTransform( tr );
  data->body.setCenterOfMassTransform( tr );
#endif
  }

void Physics::Rigid::applyForce(float x, float y, float z) {
#ifndef NO_PHYSIC
  data->body.applyForce( btVector3(x,y,z), btVector3(0,0,1) );
#endif
  }

void Physics::Rigid::update() {
#ifndef NO_PHYSIC
  btTransform n;
  data->body.getMotionState()->getWorldTransform(n);
  //data->body.activate(true);

  btMatrix3x3 m = n.getBasis();
  btVector3 v[3] = {
    m.getColumn(0),
    m.getColumn(1),
    m.getColumn(2)
    };

  btVector3 pos = n.getOrigin();

  matrix =  Tempest::Matrix4x4( v[0][0], v[0][1], v[0][2], 0,
                             v[1][0], v[1][1], v[1][2], 0,
                             v[2][0], v[2][1], v[2][2], 0,
                              pos[0],  pos[1],  pos[2], 1 );
#endif
  }

float Physics::AnimatedSphere::diameter() const {
#ifndef NO_PHYSIC
  return data->sphere.getRadius()*2;
#else
  return 0;
#endif
  }

Physics::Animated::Animated() {
  data = 0;
  engine = 0;

  x = 0;
  y = 0;
  z = 0;
  }

void Physics::Animated::setPosition(float ix, float iy, float iz) {
  x = ix;
  y = iy;
  z = iz;

  /*
  update();
  */
  }

bool Physics::Animated::isValid() const {
  return data!=0;
  }

void Physics::Animated::update() {
#ifndef NO_PHYSIC
  btTransform tr;
  data->body.getMotionState()->getWorldTransform(tr);
  tr.setOrigin( btVector3(x,y,z) );

  data->mstate.setWorldTransform( tr );
  data->body.setCenterOfMassTransform( tr );
#endif
  }


