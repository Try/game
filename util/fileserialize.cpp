#include "fileserialize.h"

#include <limits>
#include <cstdio>

#ifdef __WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <cassert>

FileSerialize::FileSerialize( const std::wstring &s, OpenMode m ) {
  f = fopen( s.data(), m==Read );
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

void FileSerialize::write(const std::wstring &val) {
  write( val.size() );
  fwrite( val.data(), sizeof(wchar_t), val.size(), f );
  }

void FileSerialize::read(std::wstring &val) {
  size_t s;
  read( s );

  if( s<100000 ){
    val.resize(s);

    fread ( &val[0], sizeof(wchar_t), val.size(), f );
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


FileSerialize::File *FileSerialize::fopen(const wchar_t *f, bool r) {
#ifdef __WIN32
  HANDLE hFile = 0;

  if( r ){
    hFile = CreateFile( f,
                        GENERIC_READ|GENERIC_WRITE,
                        FILE_SHARE_READ, NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );
    } else {
    hFile = CreateFile( f,
                        GENERIC_READ|GENERIC_WRITE,
                        FILE_SHARE_READ, NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );
    }

  return (FileSerialize::File*)hFile;
#endif
  return 0;
  }

void FileSerialize::fclose( FileSerialize::File *f ) {
#ifdef __WIN32
  CloseHandle( f );
#endif
  }

bool FileSerialize::feof( FileSerialize::File * ) const {
  return false;
  }

size_t FileSerialize::fwrite( const void * data,
                              size_t s,
                              size_t c,
                              FileSerialize::File *f) {
#ifdef __WIN32
  DWORD wmWritten = 0;
  WriteFile( f, data, s*c, &wmWritten, NULL );
  assert( s*c==wmWritten );
  return wmWritten;
#endif
  return 0;
  }

size_t FileSerialize::fread( void *data,
                             size_t s, size_t c,
                             FileSerialize::File *f) {
#ifdef __WIN32
  DWORD wmWritten = 0;
  ReadFile(f, data, s*c, &wmWritten, NULL);
  assert( s*c==wmWritten );
  return wmWritten;
#endif
  return 0;
  }
