#ifndef UPGRADE_H
#define UPGRADE_H

#include <string>
#include <vector>

struct Upgrade {
  Upgrade();

  std::string name;
  size_t id;

  int lvCount;

  struct Data{
    int buildTime;
    int gold;
    };
  std::vector<Data> data;
  };

#endif // UPGRADE_H
