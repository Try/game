#include "scene.h"

#include "algo/algo.h"

Scene::Scene() {

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

void Scene::onObjectAdded( AbstractGraphicObject *x ) {
  if( x->material().usage.shadowCast )
    shCast.push_back( x );

  if( x->material().usage.mainPass )
    mainObj.push_back( x );

  if( !x->material().glow.isEmpty() )
    glowObj.push_back( x );

  if( x->material().usage.terrainMinor )
    terrainMinor.push_back(x);

  if( x->material().usage.displace )
    displaceObj.push_back(x);

  if( x->material().usage.water )
    waterObj.push_back(x);

  if( x->material().usage.add )
    addObj.push_back(x);

  if( x->material().usage.transparent ){
    transpObj.push_back(x);
    if( x->material().zWrighting )
      transpObjZW.push_back( x );
    }

  if( x->material().usage.fogOfWar )
    fogOfWarObj.push_back(x);

  if( x->material().usage.omni )
    omniObj.push_back(x);

  if( x->material().usage.grass )
    grassObj.push_back(x);
  }

void Scene::onObjectRemoved( AbstractGraphicObject *x ) {
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
    remove( grassObj, x );
  }
