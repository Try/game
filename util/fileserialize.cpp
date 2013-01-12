#include "fileserialize.h"

#include <limits>
#include <cstdio>
#include <winsock2.h>

FileSerialize::FileSerialize( const std::string &s, OpenMode m ) {
  static const char * modes[] = {
    "rb",
    "wb",
    "wb+"
    };

  f = fopen( s.data(), modes[m] );
  mode = m;
  }

FileSerialize::~FileSerialize() {
  if( isOpen() )
    fclose(f);
  }

void FileSerialize::write(int val) {
  unsigned int x = val - std::numeric_limits<int>::min();
  write(x);
  }

void FileSerialize::read(int &val) {
  unsigned int x;
  read(x);

  val = std::numeric_limits<int>::min() + x;
  }

void FileSerialize::write(bool val) {
  write( unsigned(val) );
  }

void FileSerialize::read(bool &val) {
  unsigned v;
  read(v);

  val = v;
  }


void FileSerialize::write(unsigned int val) {
  val = htonl (val);

  fwrite( (const void*)&val, sizeof(val), 1, f );
  }

void FileSerialize::read(unsigned int &val) {
  fread( (void*)&val, sizeof(val), 1, f );
  val = ntohl(val);
  }

void FileSerialize::write( const std::string &val ) {
  write( val.size() );
  fwrite( val.data(), 1, val.size(), f );
  }

void FileSerialize::read(std::string &val) {
  size_t s;
  read( s );

  if( s<100000 ){
    val.resize(s);

    fread ( &val[0], 1, val.size(), f );
    }
  }

void FileSerialize::write(char val) {
  fwrite( &val, 1, 1, f );
  }

void FileSerialize::read(char &val) {
  fread ( &val, 1, 1, f );
  }

bool FileSerialize::isEof() const {
  return isReader() && feof(f);
  }

bool FileSerialize::isOpen() const {
  return f;
  }

bool FileSerialize::isReader() const {
  return mode==Read;
  }
