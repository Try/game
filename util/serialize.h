#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <string>
#include <vector>

class Serialize {
  public:
    enum OpenMode{
      Read   = 0,
      Write  = 1,
      Append = 2
      };

    virtual void write( unsigned int  val ) = 0;
    virtual void read ( unsigned int& val ) = 0;

    void write( int  val );
    void read ( int& val );

    void write( bool  val );
    void read ( bool& val );

    virtual void write( const std::string& val ) = 0;
    virtual void read ( std::string& val ) = 0;

    virtual void write( const std::vector<char>& val ) = 0;
    virtual void read ( std::vector<char>& val ) = 0;

    virtual void write( char  val ) = 0;
    virtual void read ( char& val ) = 0;

    virtual bool isEof() const = 0;
    virtual bool isOpen() const = 0;
    virtual bool isReader() const = 0;

    template< class T >
    Serialize & operator + ( T & t ){
      if( mode==Read )
        read(t); else
        write(t);

      return *this;
      }

  protected:
    OpenMode mode;
  };

#endif // SERIALIZE_H
