#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <Tempest/signal>
#include <Tempest/Color>
#include <Tempest/Pixmap>

#include "util/array2d.h"

class GameObject;
class ProtoObject;
class World;

class GameSerializer;

class Player {
  public:
    Player( int num );

    GameObject * editObj;

    void addUnit( GameObject * u );
    void delUnit( GameObject * u );

    int  team() const;
    void setTeam( int t );

    bool hasHostControl() const;
    void setHostCtrl( bool c );

    void select( GameObject *obj, bool s );
    std::vector<GameObject*>& selected();

    Tempest::signal< std::vector<GameObject*>&, Player& > onUnitSelected;
    Tempest::signal< GameObject&, Player& > onUnitDied;

    const Tempest::Color& color() const;

    void addGold( int g );
    int  gold() const;
    int  lim()  const;
    void addLim( int l );
    void incLim( int l );

    void setGold( int g );

    bool canBuild(const ProtoObject &p) const;

    int  limMax()  const;    

    void serialize( GameSerializer &s);
    size_t unitsCount() const;
    GameObject& unit( size_t id );

    int number() const;
    void tick( World & curW );
    void computeFog( void* curW );

    const Tempest::Pixmap& fog() const;
  private:
    struct {
      int num;
      int team;
      bool htCtrl;

      int gold, lim, limMax;
      std::vector<GameObject*> objects, selected;

      Tempest::Color color;

      Tempest::Pixmap fog;
      } m;

    void fillFog( Tempest::Pixmap &p,
                  World &wx );

    void cride( Tempest::Pixmap &p,
                int x, int y, int r);

    static bool compare( const GameObject* a,
                         const GameObject* b );
  };

#endif // PLAYER_H
