#include "serialize.h"

#include <limits>
#include <cstdio>
#include <winsock2.h>
/*
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
*/
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
