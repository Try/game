#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include "behavior/abstractbehavior.h"
#include <vector>
#include <string>
#include <memory>

#include "util/factory.h"

class GameObject;

class Behavior : public AbstractBehavior {
  public:
    struct Closure;
    Behavior();
    ~Behavior();

    bool message( Message msg, int x,
                  int y, Modifers md = BehaviorEvent::NoModifer );
    bool message( Message msg, size_t id,
                  Modifers md = BehaviorEvent::NoModifer );
    bool message( Message msg, const std::string& s,
                  Modifers md = BehaviorEvent::NoModifer );

    void tick( const Terrain & terrain );

    void bind( GameObject & gameObj,
               Closure & c );
    //void _addMoveBeavior();

    void add( const std::string & n );
    void del( AbstractBehavior* ptr );
    void clear();

    struct Closure{
      bool isOnMove, isReposMove, isMoviable, isMineralMove;
      int  colisionDisp[2];

      int  lkX, lkY;
      bool isMVLock;

      int intentToHold;
      bool isPatrul;
      };

    template< class T >
    T * find(){
      typedef Factory<AbstractBehavior, GameObject&, Behavior::Closure&> PId;
      return (T*)findex[ PId::ProductID::id<T>() ];

      /*
      for( auto  i = behaviors.begin(); i!=behaviors.end(); ++i ){
        AbstractBehavior * b = *i;

        if( dynamic_cast<T*>( b ) )
          return reinterpret_cast<T*>(b);
        }

      return 0;*/
      }

    size_t size() const{ return behaviors.size(); }
  private:
    GameObject * object;
    Closure    * clos;

    typedef std::shared_ptr<AbstractBehavior> PAbstractBehavior;
    std::vector<AbstractBehavior*> behaviors, findex;
  };

#endif // BEHAVIOR_H
