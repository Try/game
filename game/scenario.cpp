#include "scenario.h"


const std::vector<Scenario::MissionTaget> &Scenario::tagets() {
  static std::vector<Scenario::MissionTaget> tgNull;
  return tgNull;
  }
