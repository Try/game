#ifndef UNITLIST_H
#define UNITLIST_H

#include <MyWidget/Widget>

class Resource;
class GameObject;

class UnitList : public MyWidget::Widget {
  public:
    UnitList(Resource &res);

    void setup( const std::vector<GameObject *> &u );
    void onUnitDied(GameObject &obj);
  private:
    struct Btn;
    struct Lay;

    std::vector<Btn*> btn;
    Resource &res;
  };

#endif // UNITLIST_H
