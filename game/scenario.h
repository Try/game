#ifndef SCENARIO_H
#define SCENARIO_H

class GameObject;

#include <string>
#include <vector>

struct Scenario {
  virtual ~Scenario(){}

  virtual void onStartGame() {}
  virtual void tick()        {}
  virtual void onEndGame()   {}
  virtual void restartGame() {}

  virtual void onItemEvent( GameObject & obj )   {}

  struct MissionTaget{
    bool done;
    std::wstring hint;
    };

  virtual const std::vector<MissionTaget>& tagets();
  };

#endif // SCENARIO_H
