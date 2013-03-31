#ifndef TNLOPTIMIZE_H
#define TNLOPTIMIZE_H

#include "model_mx.h"
#include <vector>
#include "stlconf.h"
#include <unordered_map>

class TnlOptimize {
  public:
    TnlOptimize();

    static void index( std::vector<MVertex>& v,
                       std::vector<uint16_t>& ibo );
  };

#endif // TNLOPTIMIZE_H
