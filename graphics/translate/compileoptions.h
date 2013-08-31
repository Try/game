#ifndef COMPILEOPTIONS_H
#define COMPILEOPTIONS_H

#include <set>
#include <string>
#include <bitset>

class CompileOptions{
  public:
    CompileOptions():lang(Cg){}
    std::set<std::string> options;

    enum Lang{
      Cg,
      GLSL,
      GLSLES,
      LangCount
      };
    Lang lang;
  };

#endif // COMPILEOPTIONS_H
