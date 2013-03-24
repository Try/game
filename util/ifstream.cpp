#include "ifstream.h"

#include <Tempest/AbstractSystemAPI>

ifstream::buffer::buffer(const char *fin )
  :input( Tempest::AbstractSystemAPI::loadBytes(fin) ){
  setg(&input[0], &input[0], &input[ input.size() ]);
  }

ifstream::ifstream(const char *fin): std::istream( new buffer(fin) ) {
  }

ifstream::~ifstream() {
  delete this->rdbuf();
  }
