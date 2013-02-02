#ifndef SPELL_H
#define SPELL_H

#include <string>

struct Spell {
  public:
    Spell();

    int coolDown;
    int manaCost;

    std::string name;

    size_t id;

    enum Mode{
      CastToCoord,
      CastToUnit
      } mode;
  };

#endif // SPELL_H
