#ifndef SCENARIO_H
#define SCENARIO_H

class GameObject;

#include <string>
#include <vector>

class GameSerializer;

struct Scenario {
  virtual ~Scenario(){}

  virtual void onStartGame() {}
  virtual void tick()        {}
  virtual void onEndGame()   {}
  virtual void restartGame() {}

  virtual void onItemEvent( GameObject & obj )   { (void)obj; }

  virtual void serialize( GameSerializer & ){}

  struct MissionTaget{
    bool done;
    std::wstring hint;
    };

  virtual const std::vector<MissionTaget>& tagets();
  virtual bool isCampagin() { return true; }
  };

#endif // SCENARIO_H
