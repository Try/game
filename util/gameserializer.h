#ifndef GAMESERIALIZER_H
#define GAMESERIALIZER_H

#include "serialize.h"

class GameSerializer : public Serialize {
  public:
    GameSerializer( const std::string & s, OpenMode m );

    static const unsigned currentVersion;
    unsigned version() const;

  private:
    unsigned cversion;
  };

#endif // GAMESERIALIZER_H
