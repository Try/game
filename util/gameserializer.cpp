#include "gameserializer.h"

const unsigned GameSerializer::currentVersion = 2;

GameSerializer::GameSerializer( const std::string &s, OpenMode m )
               :Serialize(s,m){
  cversion = currentVersion;

  *this + cversion;
  }

unsigned GameSerializer::version() const {
  return cversion;
  }

