#include "serialize.h"

#include <limits>
#include <cstdio>
#include <winsock2.h>

Serialize::Serialize( const std::string &s, OpenMode m ) {
  static const char * modes[] = {
    "rb",
    "wb",
    "wb+"
    };

  f = fopen( s.data(), modes[m] );
  mode = m;
  }

Serialize::~Serialize() {
  if( isOpen() )
    fclose(f);
  }

void Serialize::write(unsigned int val) {
  val = htonl (val);

  fwrite( (const void*)&val, sizeof(val), 1, f );
  }

void Serialize::read(unsigned int &val) {
  fread( (void*)&val, sizeof(val), 1, f );
  val = ntohl(val);
  }

void Serialize::write(int val) {
  unsigned int x = val - std::numeric_limits<int>::min();
  write(x);
  }

void Serialize::read(int &val) {
  unsigned int x;
  read(x);

  val = std::numeric_limits<int>::min() + x;
  }

void Serialize::write(bool val) {
  write( unsigned(val) );
  }

void Serialize::read(bool &val) {
  unsigned v;
  read(v);

  val = v;
  }

void Serialize::write( const std::string &val ) {
  write( val.size() );
  fwrite( val.data(), 1, val.size(), f );
  }

void Serialize::read(std::string &val) {
  size_t s;
  read( s );

  if( s<100000 ){
    val.resize(s);

    fread ( &val[0], 1, val.size(), f );
    }
  }

void Serialize::write(char val) {
  fwrite( &val, 1, 1, f );
  }

void Serialize::read(char &val) {
  fread ( &val, 1, 1, f );
  }

bool Serialize::isEof() const {
  return isReader() && feof(f);
  }

bool Serialize::isOpen() const {
  return f;
  }

bool Serialize::isReader() const {
  return mode==Read;
  }
