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
