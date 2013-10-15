#include "spell.h"

Spell::Spell() {
  coolDown = 10;
  manaCost = 5;

  bulletSpeed = 600;
  mode = CastToCoord;
  autoCast = false;
  }
