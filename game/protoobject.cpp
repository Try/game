#include "protoobject.h"

ProtoObject::ProtoObject()
{
  rotateSpeed = 5;

  data.isBackground = 0;
  data.invincible   = 0;

  data.size  = 1;
  data.speed = 60;
  data.visionRange = 16;
  data.maxHp       = 100;

  data.gold      = 0;
  data.lim       = 0;
  data.limInc    = 0;
  data.buildTime = 25;

  for( int i=0; i<3; ++i ){
    sizeBounds.min[i] = 1;
    sizeBounds.max[i] = 1;
    }

  deathAnim = NoAnim;
  }

bool ProtoObject::isLandTile() const {
  const char* txpat = "land.";

  bool ok = false;
  if( name.size()>=5 ){
    ok = true;
    for( int r=0; r<5; ++r )
      if( name[r]!=txpat[r] )
        ok = false;
    }

  return ok;
  }


ProtoObject::View::View() {
  for( int i=0; i<3; ++i ){
    align[i] = 0;
    size[i] = 0;
    boxSize[i] = 0;
    }

  randRotate = 0;
  sphereDiameter = 0;
  specularFactor = 0;
  alignSize = 0;

  //isParticle = 0;
  }

const std::string& ProtoObject::GameSpecific::propStr(const std::string &s) const {
  std::map< std::string, std::string >::const_iterator i = propertyStr.find(s);
  if( i!=propertyStr.end() )
    return i->second;

  static std::string nullStr;
  return nullStr;
  }
