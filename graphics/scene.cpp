#include "scene.h"

#include "algo/algo.h"

#include <iostream>

Scene::Scene(int sz)
      :shCast(sz),
       mainObj(sz),
       glowObj(sz),
       terrainMinor(sz),
       displaceObj(sz),
       waterObj(sz),
       addObj(sz),
       transpObj(sz),
       omniObj(sz),
       grassObj(sz){
  objPtrs.push_back( &shCast );
  objPtrs.push_back( &mainObj );
  objPtrs.push_back( &glowObj );
  objPtrs.push_back( &terrainMinor );
  objPtrs.push_back( &displaceObj );
  objPtrs.push_back( &waterObj );
  objPtrs.push_back( &addObj );
  objPtrs.push_back( &transpObj );
  objPtrs.push_back( &omniObj );
  objPtrs.push_back( &grassObj );
  }

const Scene::Objects &Scene::shadowCasters() const {
  return shCast;
  }

const Scene::Objects &Scene::mainObjects() const {
  return mainObj;
  }

const Scene::Objects &Scene::glowObjects() const {
  return glowObj;
  }

const Scene::Objects &Scene::additiveObjects() const {
  return addObj;
  }

const Scene::Objects &Scene::terrainMinorObjects() const {
  return terrainMinor;
  }

const Scene::Objects &Scene::displaceObjects() const {
  return displaceObj;
  }

const Scene::Objects &Scene::waterObjects() const {
  return waterObj;
  }

const Scene::Objects &Scene::transparentObjects() const {
  return transpObj;
  }

const Scene::Objects &Scene::transparentZWObjects() const {
  return transpObjZW;
  }

const Scene::Objects &Scene::grassObjects() const {
  return grassObj;
  }

const Scene::Objects &Scene::fogOfWar() const {
  return fogOfWarObj;
  }

const Scene::Objects &Scene::omni() const {
  return omniObj;
  }

void Scene::onObjectAdded( const AbstractGraphicObject *t ) {
  onObjectOp( t, Add, t->transform() );
  }

void Scene::onObjectRemoved( const AbstractGraphicObject *t ) {
  /*
  if( x->material().usage.shadowCast )
    remove( shCast, x );

  if( x->material().usage.mainPass )
    remove( mainObj, x );

  if( !x->material().glow.isEmpty() )
    remove( glowObj, x );

  if( x->material().usage.terrainMinor )
    remove( terrainMinor, x );

  if( x->material().usage.displace )
    remove( displaceObj, x );

  if( x->material().usage.water )
    remove( waterObj, x );

  if( x->material().usage.add )
    remove( addObj, x );

  if( x->material().usage.transparent ){
    remove( transpObj, x );
    if( x->material().zWrighting )
      remove( transpObjZW, x );
    }

  if( x->material().usage.fogOfWar )
    remove( fogOfWarObj, x );

  if( x->material().usage.omni )
    remove( omniObj, x );

  if( x->material().usage.grass )
    remove( grassObj, x );*/

  onObjectOp( t, Del, t->transform() );
  }

void Scene::onObjectTransform( const AbstractGraphicObject *t,
                               const MyGL::Matrix4x4 &mat ) {
  onObjectOp( t, RePos, mat );
  //onObjectAdded(t);
  }

void Scene::onObjectOp( const AbstractGraphicObject *t,
                        Act ins,
                        const MyGL::Matrix4x4 &mat ) {
  if( t->material().usage.shadowCast )
    onObjectOp( shCast, t, ins, mat );

  if( t->material().usage.mainPass )
    onObjectOp( mainObj, t, ins, mat );

  if( !t->material().glow.isEmpty() )
    onObjectOp( glowObj, t, ins, mat );

  if( t->material().usage.terrainMinor )
    onObjectOp( terrainMinor, t, ins, mat );

  if( t->material().usage.displace )
    onObjectOp( displaceObj, t, ins, mat );

  if( t->material().usage.water )
    onObjectOp( waterObj, t, ins, mat );

  if( t->material().usage.add )
    onObjectOp( addObj, t, ins, mat );

  if( t->material().usage.transparent ){
    onObjectOp( transpObj, t, ins, mat );
    if( t->material().zWrighting )
      onObjectOp( transpObjZW, t, ins, mat );
    }

  if( t->material().usage.fogOfWar )
    onObjectOp( fogOfWarObj, t, ins, mat );

  if( t->material().usage.omni )
    onObjectOp( omniObj, t, ins, mat );

  if( t->material().usage.grass )
    onObjectOp( grassObj, t, ins, mat );
  }

void Scene::onObjectOp( Scene::Objects &obj,
                        const AbstractGraphicObject *t,
                        Act ins,
                        const MyGL::Matrix4x4 &mat ) {

  float x = mat.at(3,0);
  float y = mat.at(3,1);
  float z = mat.at(3,2);

  float dx = t->bounds().mid[0]*t->sizeX(),
        dy = t->bounds().mid[1]*t->sizeY(),
        dz = t->bounds().mid[2]*t->sizeZ();

  if( ins==Add ){
    insert( obj, t, x+dx, y+dy, z+dz );
    } else
  if( ins==Del ){
    remove( obj, t, x+dx, y+dy, z+dz );
    } else
  if( ins==RePos ){
    obj.reposition( t, x+dx, y+dy, z+dz, t->radius() );
    }
  }

void Scene::remove( Scene::Objects &obj,
                    const AbstractGraphicObject *t,
                    float x, float y, float z ) {
  //std::cout << "remove( " << t <<","  << x <<", " << y <<", " << z <<" )" << std::endl;
  obj.remove(t, x, y, z);
  }

void Scene::insert( Scene::Objects &obj,
                    const AbstractGraphicObject *t,
                    float x, float y, float z) {
  //std::cout << "insert( " << t <<"," << x <<", " << y <<", " << z <<" )" << std::endl;
  obj.insert(t, x, y, z, t->radius() );
  }

