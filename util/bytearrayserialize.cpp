#include "bytearrayserialize.h"

#include <limits>
#include <cstdio>

#ifdef __WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

ByteArraySerialize::ByteArraySerialize( std::vector<char> &io,
                                        Serialize::OpenMode m ) {
  f  = &io;
  cf = &io;

  mode = m;
  pos  = 0;
  }

ByteArraySerialize::ByteArraySerialize(const std::vector<char> &io) {
  f  =   0;
  cf = &io;

  mode = Read;
  pos  = 0;
  }

ByteArraySerialize::~ByteArraySerialize() {
  }

void ByteArraySerialize::write(int val) {
  unsigned int x = val - std::numeric_limits<int>::min();
  write(x);
  }

void ByteArraySerialize::read(int &val) {
  unsigned int x;
  read(x);

  val = std::numeric_limits<int>::min() + x;
  }

void ByteArraySerialize::write(bool val) {
  write( unsigned(val) );
  }

void ByteArraySerialize::read(bool &val) {
  unsigned v;
  read(v);

  val = v;
  }


void ByteArraySerialize::write(unsigned int val) {
  val = htonl (val);

  fwrite( (const void*)&val, sizeof(val), 1, f );
  }

void ByteArraySerialize::read(unsigned int &val) {
  fread( (void*)&val, sizeof(val), 1, cf );
  val = ntohl(val);
  }

void ByteArraySerialize::write( const std::string &val ) {
  write( val.size() );
  fwrite( val.data(), 1, val.size(), f );
  }

void ByteArraySerialize::read(std::string &val) {
  size_t s;
  read( s );

  if( s<100000 ){
    val.resize(s);

    fread ( &val[0], 1, val.size(), cf );
    }
  }

void ByteArraySerialize::write(const std::vector<char> &val) {
  write( val.size() );
  fwrite( val.data(), 1, val.size(), f );
  }

void ByteArraySerialize::read(std::vector<char> &val) {
  size_t s;
  read( s );

  if( s<100000 ){
    val.resize(s);

    fread ( &val[0], 1, val.size(), cf );
    }
  }

void ByteArraySerialize::write(char val) {
  fwrite( &val, 1, 1, f );
  }

void ByteArraySerialize::read(char &val) {
  fread ( &val, 1, 1, cf );
  }

bool ByteArraySerialize::isEof() const {
  return isReader() && (pos >= cf->size());
  }

bool ByteArraySerialize::isOpen() const {
  return f;
  }

bool ByteArraySerialize::isReader() const {
  return mode==Read;
  }

void ByteArraySerialize::fwrite( const void * x , size_t esz,
                                 size_t sz, std::vector<char> *f ) {
  sz *= esz;
  const char* w = (const char*)x;
  for( size_t i=0; i<sz; ++i )
    f->push_back( w[i] );
  }

void ByteArraySerialize::fread(  void * x , size_t esz,
                                 size_t sz, const std::vector<char> *f ) {
  sz *= esz;
  char* r = (char*)x;

  for( size_t i=0; i<sz; ++i ){
    r[i] = (*f)[pos];

    if( pos+1 < f->size() )
      ++pos;
    }
  }
