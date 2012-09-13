#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <MyWidget/signal>
#include <MyGL/Color>

class GameObject;

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

    MyWidget::signal< std::vector<GameObject*>& > onUnitSelected;

    const MyGL::Color& color() const;
  private:
    struct {
      int num;
      int team;
      bool htCtrl;
      std::vector<GameObject*> objects, selected;

      MyGL::Color color;
      } m;
  };

#endif // PLAYER_H
