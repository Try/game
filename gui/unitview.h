#ifndef UNITVIEW_H
#define UNITVIEW_H

#include "textureview.h"

#include <MyGL/Texture2d>
#include <MyGL/Scene>

#include <memory>

#include "game/gameobjectview.h"

class UnitView : public TextureView {
  public:
    UnitView( Resource & res );
    ~UnitView();

    MyWidget::signal< const MyGL::Scene &,
                      MyGL::Texture2d & > renderScene;

    void setupUnit( GameObject * obj );
  protected:
    void paintEvent(MyWidget::PaintEvent &e);

    MyGL::Scene   scene;
    Resource    & res;

    std::unique_ptr<GameObjectView> view;

    void setupCamera();
  };

#endif // UNITVIEW_H
