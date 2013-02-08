#ifndef FILESERIALIZE_H
#define FILESERIALIZE_H

#include <cstdio>
#include <string>

#include "serialize.h"

class FileSerialize : public Serialize {
  public:
    FileSerialize(const std::wstring &s, OpenMode m );
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
    //FILE * f;

    struct File;
    File * f;

    File *fopen(const wchar_t*, bool r );
    void  fclose( File *  );
    bool  feof( File *  ) const;

    size_t fwrite( const void*, size_t, size_t, File* f );
    size_t fread ( void*,       size_t, size_t, File* f);
};

#endif // FILESERIALIZE_H
