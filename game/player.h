#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <Tempest/signal>
#include <Tempest/Color>
#include <Tempest/Pixmap>

#include "util/array2d.h"
#include <memory>

class GameObject;
class ProtoObject;
class Upgrade;
class World;

class GameSerializer;

class Player {
  public:
    Player(int num );
    virtual ~Player();

    static const Tempest::Color colors[8];
    GameObject * editObj;

    void addUnit( GameObject * u );
    void delUnit( GameObject * u );

    int  team() const;
    void setTeam( int t );

    bool hasHostControl() const;
    void setHostCtrl( bool c );

    bool isAi() const;
    void setAsAI( bool a );

    virtual void select( GameObject *obj, bool s );
    virtual std::vector<GameObject*>& selected();

    Tempest::signal< std::vector<GameObject*>&, Player& > onUnitSelected;
    Tempest::signal< GameObject&, Player& > onUnitDied;

    const Tempest::Color& color() const;
    void setColor( const Tempest::Color & cl );

    void addGold( int g );
    int  gold() const;
    int  lim()  const;
    void addLim( int l );
    void incLim( int l );

    void setGold( int g );

    bool canBuild(const ProtoObject &p) const;
    bool canBuild(const Upgrade &p) const;

    int  limMax()  const;    

    void serialize( GameSerializer &s);
    size_t unitsCount() const;
    GameObject& unit( size_t id );

    int number() const;
    void tick( World & curW );
    void computeFog( void* curW );

    const Tempest::Pixmap& fog() const;

    int  atackGrade( size_t atype ) const;
    int  gradeLv( const Upgrade &atype ) const;
    void mkGrade( const Upgrade& atype );
  protected:
    struct {
      int num;
      int team;
      bool htCtrl;
      bool isAi;

      int gold, lim, limMax;
      std::vector<GameObject*> objects, selected;

      Tempest::Color color;

      Tempest::Pixmap fog;

      std::vector<int> gradeLv;
      } m;

    void fillFog( Tempest::Pixmap &p,
                  World &wx );

    void cride( Tempest::Pixmap &p,
                int x, int y, int r);

    static bool compare( const GameObject* a,
                         const GameObject* b );
  };

#endif // PLAYER_H
