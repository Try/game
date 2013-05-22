#ifndef BYTEARRAYSERIALIZE_H
#define BYTEARRAYSERIALIZE_H

#include <vector>
#include <string>

#include "serialize.h"

class ByteArraySerialize : public Serialize {
  public:
    ByteArraySerialize( std::vector<char> & io, OpenMode m );
    ByteArraySerialize( const std::vector<char> & io );
    ~ByteArraySerialize();

    ByteArraySerialize( const ByteArraySerialize&  )              = delete;
    ByteArraySerialize & operator = ( const ByteArraySerialize& ) = delete;

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

    void write( char  val );
    void read ( char& val );

    bool isEof() const;
    bool isOpen() const;
    bool isReader() const;

    template< class T >
    ByteArraySerialize & operator + ( T & t ){
      if( mode==Read )
        read(t); else
        write(t);

      return *this;
      }

  private:
    std::vector<char> * f;
    const std::vector<char> * cf;
    size_t pos;

    void fwrite(const void *x, size_t esz, size_t sz, std::vector<char> * f);
    void  fread(void *x, size_t esz, size_t sz, const std::vector<char> *f);
};

#endif // BYTEARRAYSERIALIZE_H
