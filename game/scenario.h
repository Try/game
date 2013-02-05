#ifndef SCENARIO_H
#define SCENARIO_H

class GameObject;

struct Scenario {
  virtual ~Scenario(){}

  virtual void onStartGame() {}
  virtual void tick()        {}
  virtual void onEndGame()   {}

  virtual void onItemEvent( GameObject & obj )   {}
  };

#endif // SCENARIO_H
