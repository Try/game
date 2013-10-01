#ifndef GAMESERIALIZER_H
#define GAMESERIALIZER_H

#include "fileserialize.h"

class GameSerializer : public FileSerialize {
  public:
    GameSerializer( const std::wstring & s, OpenMode m );
    GameSerializer( const std::wstring & s );

    static const unsigned currentVersion;
    unsigned version() const;

  private:
    unsigned cversion;
  };

#endif // GAMESERIALIZER_H
