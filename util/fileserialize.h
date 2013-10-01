#ifndef FILESERIALIZE_H
#define FILESERIALIZE_H

#include <cstdio>
#include <string>
#include <vector>

#include "serialize.h"

class FileSerialize : public Serialize {
  public:
    FileSerialize(const std::wstring &s, OpenMode m );
    FileSerialize(const std::string &s);
    ~FileSerialize();

    FileSerialize( const FileSerialize&  )              = delete;
    FileSerialize & operator = ( const FileSerialize& ) = delete;

    void write( unsigned int  val );
    void read ( unsigned int& val );

    void write( int  val );
    void read ( int& val );

    void write( bool  val );
    void read ( bool& val );

    void write( const std::string& val );
    void read ( std::string& val );

    void write( const std::vector<char>& val );
    void read ( std::vector<char>& val );

    void write( const std::wstring& val );
    void read ( std::wstring& val );

    void write( char  val );
    void read ( char& val );

    bool isEof() const;
    bool isOpen() const;
    bool isReader() const;

    template< class T >
    FileSerialize & operator + ( T & t ){
      if( mode==Read )
        read(t); else
        write(t);

      return *this;
      }

  private:
    std::wstring fname;
    std::vector<char> data;
    size_t rpos;

    size_t fwrite( const void*, size_t, size_t );
    size_t fread ( void*,       size_t, size_t );
};

#endif // FILESERIALIZE_H
