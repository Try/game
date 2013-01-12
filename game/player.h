#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <MyWidget/signal>
#include <MyGL/Color>

class GameObject;
class ProtoObject;

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

    bool isInSync() const;
    void setSyncFlag( bool s );
  private:
    struct {
      int num;
      int team;
      bool htCtrl;
      bool sync;

      int gold, lim, limMax;
      std::vector<GameObject*> objects, selected;

      MyGL::Color color;
      } m;

    static bool compare( const GameObject* a,
                         const GameObject* b );
  };

#endif // PLAYER_H
