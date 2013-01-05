#ifndef COMMANDSPANEL_H
#define COMMANDSPANEL_H

#include "gui/panel.h"

#include <MyWidget/Layout>

#include "game/protoobject.h"

class GameObject;
class BehaviorMSGQueue;

class CommandsPanel : public Panel {
  public:
    CommandsPanel(Resource &res, BehaviorMSGQueue & q );

    void bind(GameObject *u );

    class Layout: public MyWidget::Layout{
      void applyLayout();
      };

    MyWidget::signal<> onPageCanged;
private:
    Resource & res;
    BehaviorMSGQueue & msg;

    class BtnBase;
    class BuyBtn;
    class PageBtn;

    int pl;
    GameObject * u0;

    void bind( const ProtoObject::Commans::Page &u );
    void buyEvent( const std::string& unit );
    void moveClick();
    void stopClick();
    void atkClick();

    void setPage( int p );

    void bindStartPage(const ProtoObject::Commans::Page *p );
    void bindPage( const ProtoObject::Commans::Page &p );
  };

#endif // COMMANDSPANEL_H
