#include "fileserialize.h"

#include <limits>
#include <cstdio>

#include <Tempest/SystemAPI>

#ifdef __WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <Tempest/Assert>

FileSerialize::FileSerialize( const std::wstring &s, OpenMode m ) {
  fname = s;

  mode = m;
  rpos = 0;

  if( mode==Read )
    data = Tempest::SystemAPI::loadBytes(s.data());
  }

FileSerialize::FileSerialize(const std::string &s) {
  fname.assign( s.begin(), s.end() );

  mode = Read;
  rpos = 0;

  if( mode==Read )
    data = Tempest::SystemAPI::loadBytes(s.data());
  }

FileSerialize::~FileSerialize() {
  if( mode==Write )
    Tempest::SystemAPI::writeBytes(fname.c_str(), data);
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

  fwrite( (const void*)&val, sizeof(val), 1 );
  }

void FileSerialize::read(unsigned int &val) {
  fread( (void*)&val, sizeof(val), 1 );
  val = ntohl(val);
  }

void FileSerialize::write( const std::string &val ) {
  write( val.size() );
  fwrite( val.data(), 1, val.size() );
  }

void FileSerialize::read(std::string &val) {
  size_t s;
  read( s );

  if( s<100000 ){
    val.resize(s);

    fread ( &val[0], 1, val.size() );
    }
  }

void FileSerialize::write(const std::vector<char> &val) {
  write( val.size() );
  fwrite( val.data(), 1, val.size() );
  }

void FileSerialize::read(std::vector<char> &val) {
  size_t s;
  read( s );

  if( s<100000 ){
    val.resize(s);

    fread ( &val[0], 1, val.size() );
    }
  }

void FileSerialize::write(const std::wstring &val) {
  write( val.size() );
  fwrite( val.data(), sizeof(wchar_t), val.size() );
  }

void FileSerialize::read(std::wstring &val) {
  size_t s;
  read( s );

  if( s<100000 ){
    val.resize(s);

    fread ( &val[0], sizeof(wchar_t), val.size() );
    }
  }

void FileSerialize::write(char val) {
  fwrite( &val, 1, 1 );
  }

void FileSerialize::read(char &val) {
  fread ( &val, 1, 1 );
  }

bool FileSerialize::isEof() const {
  return isReader() && rpos==data.size();
  }

bool FileSerialize::isOpen() const {
  return data.size();
  }

bool FileSerialize::isReader() const {
  return mode==Read;
  }

size_t FileSerialize::fwrite(const void * c_data,
                              size_t s,
                              size_t c) {
  size_t sz = s*c;
  for( size_t i=0; i<sz; ++i )
    data.push_back( ((const char*)c_data)[i] );

  return 0;
  }

size_t FileSerialize::fread(void *odata,
                             size_t s, size_t c) {
  if( data.size() ){
    s *= c;
    while( s ){
      *((char*)odata) = data[rpos];
      ++rpos;
      ++((char*&)odata);
      --s;
      }

    return s*c;
    }

  return 0;
  }
