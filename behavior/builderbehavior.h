#ifndef BUILDERBEHAVIOR_H
#define BUILDERBEHAVIOR_H

#include "abstractbehavior.h"
#include "behavior.h"
#include "gui/inputhook.h"
#include "graphics/material.h"

#include <MyGL/GraphicObject>

class ProtoObject;

class BuilderBehavior : public AbstractBehavior  {
  public:
    BuilderBehavior( GameObject & obj,
                     Behavior::Closure & c );
    ~BuilderBehavior();


    void tick( const Terrain & terrain );
    bool message(Message msg, const std::string &s, Modifers md);
    bool message(Message msg, int x, int y, Modifers md);
  private:
    GameObject & obj;
    GraphicObject hud, hudIntent;
    const ProtoObject* proto;

    InputHook hook;
    bool instaled;
    std::string taget;

    void mouseDown( MyWidget::MouseEvent& e );
    void mouseUp  ( MyWidget::MouseEvent& e );

    void mouseMove( MyWidget::MouseEvent& e );

    void move(int x, int y);
    void build();
    void onRemoveHook();

    struct CreateOrder{
      std::string taget;
      int x,y;
      };

    std::vector<CreateOrder> tasks;
    MyGL::Texture2d red, blue;
  };

#endif // BUILDERBEHAVIOR_H
