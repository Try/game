#include "upgrade.h"

Upgrade::Upgrade() {
  lvCount = 1;
  id      = -1;

  data.resize(1);
  data[0].buildTime = 10;
  data[0].gold      = 50;
  }
