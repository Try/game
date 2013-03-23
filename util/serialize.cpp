#include "serialize.h"

#include <limits>
#include <cstdio>

#ifdef __WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

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
