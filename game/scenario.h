#ifndef SCENARIO_H
#define SCENARIO_H

class Scenario {
  public:
    virtual ~Scenario(){}

    virtual void onStartGame() {}
    virtual void tick()        {}
    virtual void onEndGame()   {}
  };

#endif // SCENARIO_H
