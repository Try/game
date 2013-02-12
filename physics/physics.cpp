#include "physics.h"

// #include <tokamak.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <iostream>

#include "game/world.h"

#include <memory>

struct Physics::Data{
  std::unique_ptr<btBroadphaseInterface>               broadphase;
  std::unique_ptr<btDefaultCollisionConfiguration>     collisionConfiguration;
  std::unique_ptr<btCollisionDispatcher>               dispatcher;
  std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

  std::unique_ptr<Physics::TerrainBody>                terrain;
  std::unique_ptr<btDiscreteDynamicsWorld>             dynamicsWorld;

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
  };

struct Physics::RigidBody{
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
  };

struct Physics::AnimatedBody:public Physics::RigidBody {
  AnimatedBody( float x, float y, float z, float d ): RigidBody(x,y,z,d,0){}
  AnimatedBody( float  x, float  y, float  z,
                float sx, float sy, float sz ): RigidBody(x,y,z,sx,sy,sz,0){}
  };

struct Physics::TerrainBody{
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
  };


Physics::Physics(int /*tw*/, int /*th*/) {
  data = new Data();
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
  }

Physics::~Physics() {
  if( data->terrain )
    data->dynamicsWorld->removeRigidBody( &data->terrain->body );

  delete data;
  }

void Physics::tick() {
  data->dynamicsWorld->stepSimulation( 1/60.f, 10 );
  }

void Physics::setTerrain( const Terrain &t ) {
  std::vector<float> heightMap( t.width()*t.height() );

  float min = World::coordCast( t.at(0,0) ), max = min;

  for( int i=0; i<t.width(); ++i )
    for( int r=0; r<t.height(); ++r ){
      float h = World::coordCast( t.heightAt(i,r) );
      heightMap[ i+r*t.width() ] = h;

      min = std::min(min, h);
      max = std::max(max, h);
      }

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

  //w->;
  }


Physics::Sphere Physics::createSphere( float x, float y, float z, float d ) {
  Physics::Sphere s;
  s.data = new RigidBody( x, y, z, d, 0.1 );
  data->dynamicsWorld->addRigidBody( &s.data->body );
  return s;
  }

void Physics::free( Physics::Rigid &s) {
  if( s.data ){
    data->dynamicsWorld->removeRigidBody( &s.data->body );
    --data->rigidCount;
    s.data = 0;
    }
  }

Physics::Box Physics::createBox(float x, float y, float z,
                                float sx, float sy, float sz) {
  Physics::Box s;
  s.data = new RigidBody( x, y, z, sx, sy, sz, 0.1 );
  data->dynamicsWorld->addRigidBody( &s.data->body );
  return s;
  }

Physics::AnimatedSphere Physics::createAnimatedSphere( float x, float y,
                                                       float z, float d ){
  Physics::AnimatedSphere s;
  s.data = new AnimatedBody( x, y, z, d );
  data->dynamicsWorld->addRigidBody( &s.data->body );
  return s;
  }

Physics::AnimatedBox Physics::createAnimatedBox( float  x, float  y, float  z,
                                                 float sx, float sy, float sz ){
  Physics::AnimatedBox s;
  s.data = new AnimatedBody( x, y, z, sx, sy, sz );
  data->dynamicsWorld->addRigidBody( &s.data->body );
  return s;
  }

void Physics::free(Physics::AnimatedSphere &s) {
  if( s.data ){
    data->dynamicsWorld->removeRigidBody( &s.data->body );
    --data->animCount;
    s.data = 0;
    }
  }

void Physics::free(Physics::AnimatedBox &s) {
  if( s.data ){
    data->dynamicsWorld->removeRigidBody( &s.data->body );
    --data->animCount;
    s.data = 0;
    }
  }

Physics::Rigid::Rigid() {
  data = 0;
  }

float Physics::Rigid::x() const {
  btTransform trans;
  data->body.getMotionState()->getWorldTransform(trans);

  return trans.getOrigin().x();
  }

float Physics::Rigid::y() const {
  btTransform trans;
  data->body.getMotionState()->getWorldTransform(trans);

  return trans.getOrigin().y();
  }

float Physics::Rigid::z() const {
  btTransform trans;
  data->body.getMotionState()->getWorldTransform(trans);

  return trans.getOrigin().z();
  }

MyGL::Matrix4x4 Physics::Rigid::transform() {
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

  return MyGL::Matrix4x4( v[0][0], v[0][1], v[0][2], 0,
                          v[1][0], v[1][1], v[1][2], 0,
                          v[2][0], v[2][1], v[2][2], 0,
                           pos[0],  pos[1],  pos[2], 1 );
  }

float Physics::Rigid::diameter() const {
  return data->sphere.getRadius()*2;
  }

bool Physics::Rigid::isValid() const {
  return data!=0;
  }

void Physics::Rigid::activate() {
  data->body.activate(true);
  }

bool Physics::Rigid::isActive() {
  return data->body.isActive();
  }

void Physics::Rigid::setPosition(float x, float y, float z) {
  btTransform tr;
  data->body.getMotionState()->getWorldTransform(tr);
  tr.setOrigin( btVector3(x,y,z) );

  data->mstate.setWorldTransform( tr );
  data->body.setCenterOfMassTransform( tr );
  }

void Physics::Rigid::setAngle(float rx, float ry) {
  rx = rx*M_PI/180.0;
  ry = ry*M_PI/180.0;

  btTransform tr;
  data->body.getMotionState()->getWorldTransform(tr);
  tr.setRotation( btQuaternion(0, rx, ry));

  data->mstate.setWorldTransform( tr );
  data->body.setCenterOfMassTransform( tr );
  }

void Physics::Rigid::applyForce(float x, float y, float z) {
  data->body.applyForce( btVector3(x,y,z), btVector3(0,0,1) );
  }

float Physics::AnimatedSphere::diameter() const {
  return data->sphere.getRadius()*2;
  }

Physics::Animated::Animated() {
  data = 0;
  }

void Physics::Animated::setPosition(float x, float y, float z) {
  btTransform tr;
  data->body.getMotionState()->getWorldTransform(tr);
  tr.setOrigin( btVector3(x,y,z) );

  data->mstate.setWorldTransform( tr );
  data->body.setCenterOfMassTransform( tr );
  }

bool Physics::Animated::isValid() const {
  return data!=0;
  }


