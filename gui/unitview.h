#ifndef UNITVIEW_H
#define UNITVIEW_H

#include "textureview.h"

#include <MyGL/Texture2d>
#include <memory>

#include "game/gameobjectview.h"
#include "graphics/particlesystemengine.h"

#include "game/world.h"

class Game;

class UnitView : public TextureView {
  public:
    UnitView( Resource & res );
    ~UnitView();

    MyWidget::signal< const Scene &,
                      ParticleSystemEngine &,
                      MyGL::Texture2d & > renderScene;

    void setupUnit( GameObject * obj );
    void setupUnit( Game &game,
                    const std::string & proto );
    void updateView();

    MyWidget::signal<GameObject&> setCameraPos;
  protected:
    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);

    void paintEvent(MyWidget::PaintEvent &e);

    Scene    scene;
    Resource & res;

    bool  folowMode, rotateMode;
    float rotAngle;
    GameObject * curUnit;

    std::unique_ptr<World> world;
    std::unique_ptr<ParticleSystemEngine> pEng;
    std::unique_ptr<GameObjectView> view;

    void setupCamera();

    void resizeEvent( int x, int y );
  };

#endif // UNITVIEW_H
