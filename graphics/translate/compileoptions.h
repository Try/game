#ifndef COMPILEOPTIONS_H
#define COMPILEOPTIONS_H

#include <set>
#include <string>
#include <bitset>

class CompileOptions{
  public:
    CompileOptions():lang(Cg), maxVaryings(8), atestRef(0){}
    std::set<std::string> options;

    enum Lang{
      Cg,
      GLSL,
      GLSLES,
      LangCount
      };
    Lang lang;
    int  maxVaryings;
    float atestRef;
  };

#endif // COMPILEOPTIONS_H
