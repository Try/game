#ifndef LEXICALCAST_H
#define LEXICALCAST_H

#include <string>
#include <sstream>

class Lexical {
  public:
    template< class T >
    static T cast( const std::string& src ){
      std::istringstream s(src);
      T val;
      s >> val;

      return val;
      }

    template< class T >
    static std::string upcast( const T& src ){
      std::ostringstream s;
      s << src;

      return s.str();
      }
  };

#endif // LEXICALCAST_H
