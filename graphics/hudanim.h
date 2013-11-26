#ifndef HUDANIM_H
#define HUDANIM_H

#include "game/gameobjectview.h"

class EfectBase{
  public:
    enum UnitEfect{
      NoEfect = 0,
      Strorm
      };

    virtual ~EfectBase(){}
    virtual void tick(){}
    virtual bool isEnd() const { return true; }
  };

class HudAnim : public EfectBase {
  public:
    HudAnim( Scene & s,
             World       & wrld,
             const ProtoObject &p,
             const PrototypesLoader & pl,

             Resource & res,
             Physics  & phys );

    HudAnim& setPosition( float x, float y, float z );

    void tick();
    void setTimeout( int t );
    bool isEnd() const;
  private:
    GameObjectView view;

    int t;
  };

class StormEfect : public EfectBase {
  public:
    StormEfect( Scene & s,
                World       & wrld,
                const ProtoObject &p,
                const PrototypesLoader & pl,

                Resource & res,
                Physics  & phys );

    void setPosition( int x, int y, float z );

    void tick();
    bool isEnd() const;
  private:
    int t;
    World &w;
    int x,y;

    static void findEnemy(GameObject& tg, int x, int y, int r, int team);
  };

class ForceFieldEfect : public EfectBase {
  public:
    ForceFieldEfect( Scene & s,
                     World       & wrld,
                     const ProtoObject &p,
                     const PrototypesLoader & pl,

                     Resource & res,
                     Physics  & phys );
    ~ForceFieldEfect();

    void setPosition( int x, int y, float z );

    void tick();
    bool isEnd() const;
  private:
    World &w;
    int x,y;

    int t;
    GameObjectView view;
  };

#endif // HUDANIM_H
