#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <string>

class Serialize {
  public:
    enum OpenMode{
      Read   = 0,
      Write  = 1,
      Append = 2
      };

    Serialize( const std::string & s, OpenMode m );
    ~Serialize();

    Serialize( const Serialize&  )              = delete;
    Serialize & operator = ( const Serialize& ) = delete;

    void write( unsigned int  val );
    void read ( unsigned int& val );

    void write( int  val );
    void read ( int& val );

    void write( const std::string& val );
    void read ( std::string& val );

    void write( char  val );
    void read ( char& val );

    bool isEof() const;
    bool isReader() const;

    template< class T >
    Serialize & operator + ( T & t ){
      if( mode==Read )
        read(t); else
        write(t);

      return *this;
      }
  private:
    FILE * f;

    OpenMode mode;
  };

#endif // SERIALIZE_H
