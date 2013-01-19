#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <MyWidget/signal>
#include <MyGL/Color>
#include <MyGL/Pixmap>

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

    MyWidget::signal< std::vector<GameObject*>&, Player& > onUnitSelected;
    MyWidget::signal< GameObject&, Player& > onUnitDied;

    const MyGL::Color& color() const;

    void addGold( int g );
    int  gold() const;
    int  lim()  const;
    void addLim( int l );
    void incLim( int l );

    bool canBuild(const ProtoObject &p) const;

    int  limMax()  const;    

    void serialize( GameSerializer &s);
    size_t unitsCount() const;
    GameObject& unit( size_t id );

    int number() const;
    void tick( World & curW );

    const MyGL::Pixmap& fog() const;
  private:
    struct {
      int num;
      int team;
      bool htCtrl;

      int gold, lim, limMax;
      std::vector<GameObject*> objects, selected;

      MyGL::Color color;
      MyGL::Pixmap fog;
      } m;

    void fillFog( MyGL::Pixmap &p, World &wx );
    void cride(MyGL::Pixmap &p, int x, int y, int r);

    static bool compare( const GameObject* a,
                         const GameObject* b );
  };

#endif // PLAYER_H
