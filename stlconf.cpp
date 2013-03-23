#include "stlconf.h"

#include <cassert>

#ifdef __ANDROID__

namespace std{
//exception::exception() {}
//exception::~exception() {}
//const char* exception::what() const {}
}

namespace boost{
  void throw_exception( const std::exception& ){
    assert(0);
    }
  }

#endif
