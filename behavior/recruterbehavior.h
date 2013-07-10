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

    struct Queue{
      int time;
      enum OrderType{
        Unit,
        Upgrade
        };

      struct Order{
        OrderType   type;
        std::string name;
        int         time;

        std::string icon;
        };

      void addUnit( const std::string & s,
                    int time ){
        orders.resize( orders.size()+1 );
        orders.back().name = s;
        orders.back().type = Unit;
        orders.back().time = time;
        orders.back().icon = "gui/icon/"+s;
        }

      void addUpgrade( const std::string & s,
                       int time ){
        orders.resize( orders.size()+1 );
        orders.back().name = s;
        orders.back().type = Upgrade;
        orders.back().time = time;
        orders.back().icon = "gui/icon/"+s;
        }

      std::vector<Order> orders;
      };
    const std::vector<Queue>& orders() const;
  private:
    GameObject & obj;
    WeakWorldPtr taget;

    int rallyX, rallyY;

    std::vector<Queue> queue;

    int  minQtime();
    int  qtime(const Queue &q) const;

    bool queueLim;
    bool create(const Queue::Order &ord, const Terrain & terrain );
    bool createUpgrade(const std::string& s);
    bool createUnit( const std::string& s, const Terrain & terrain );

    GameObjectView light, flag;

    size_t minQueueSize() const;
    size_t maxQueueSize() const;

    int limOf( const Queue::Order &o );
  };

#endif // RECRUTERBEHAVIOR_H
