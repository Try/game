#include "gameserializer.h"

const unsigned GameSerializer::currentVersion = 7;

GameSerializer::GameSerializer(const std::wstring &s, OpenMode m )
               :FileSerialize(s,m){
  cversion = currentVersion;

  *this + cversion;
  }

unsigned GameSerializer::version() const {
  return cversion;
  }

