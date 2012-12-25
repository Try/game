#include "protoobject.h"

ProtoObject::ProtoObject()
{
  rotateSpeed = 5;

  data.isBackground = 0;

  data.size  = 1;
  data.speed = 15;
  data.visionRange = 14;
  data.maxHp       = 100;

  data.gold      = 0;
  data.lim       = 0;
  data.limInc    = 0;
  data.buildTime = 25;
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
  }
