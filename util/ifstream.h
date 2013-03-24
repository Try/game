#ifndef IFSTREAM_H
#define IFSTREAM_H

#include "stlconf.h"
#include <istream>
#include <vector>

class ifstream : public std::istream {
  public:
    ifstream( const char* fin );
    ~ifstream();

  private:
    class buffer : public std::streambuf {
      public:
        buffer( const char *f );

        int_type underflow() {
          return  gptr() == egptr() ?
                traits_type::eof() :
                traits_type::to_int_type(*gptr());
          }
        std::vector<char> input;
      };

    //buffer buf;
  };

#endif // IFSTREAM_H
