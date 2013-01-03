#ifndef UNITVIEW_H
#define UNITVIEW_H

#include "textureview.h"

#include <MyGL/Texture2d>
#include <MyGL/Scene>

#include <memory>

#include "game/gameobjectview.h"
#include "graphics/particlesystemengine.h"

class UnitView : public TextureView {
  public:
    UnitView( Resource & res );
    ~UnitView();

    MyWidget::signal< const MyGL::Scene &,
                      ParticleSystemEngine &,
                      MyGL::Texture2d & > renderScene;

    void setupUnit( GameObject * obj );
    void updateView();

    MyWidget::signal<GameObject&> setCameraPos;
  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);

    MyGL::Scene   scene;
    Resource    & res;

    bool folowMode;
    GameObject * curUnit;

    std::unique_ptr<ParticleSystemEngine> pEng;
    std::unique_ptr<GameObjectView> view;

    void setupCamera();
  };

#endif // UNITVIEW_H
