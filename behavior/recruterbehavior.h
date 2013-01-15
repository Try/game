#ifndef RECRUTERBEHAVIOR_H
#define RECRUTERBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"

#include "game/gameobjectview.h"
#include "util/weakworldptr.h"

class WorkerBehavior;

class RecruterBehavior : public AbstractBehavior  {
  public:
    RecruterBehavior( GameObject & obj,
                      Behavior::Closure & c );
    ~RecruterBehavior();


    void tick( const Terrain & terrain );
    bool message(Message msg, const std::string &cls, Modifers md);
    bool message(Message msg, size_t id, Modifers md);
    bool message(Message msg, int x, int y, Modifers md);

    int  qtime();
    int  ctime() const;
    const std::vector<std::string>& orders() const;
  private:
    GameObject & obj;
    WeakWorldPtr taget;

    int time, rallyX, rallyY;
    std::vector<std::string> queue;

    bool queueLim;
    bool create( const std::string& s, const Terrain & terrain );

    GameObjectView light, flag;
  };

#endif // RECRUTERBEHAVIOR_H
