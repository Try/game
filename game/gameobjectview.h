#ifndef GAMEOBJECTVIEW_H
#define GAMEOBJECTVIEW_H

#include "physics/physics.h"
#include <MyGL/GraphicObject>
#include "envobject.h"
#include "game/protoobject.h"
#include "graphics/particlesystem.h"
class SmallGraphicsObject;

#include <memory>

class Resource;
class World;
class PrototypesLoader;
class GameObject;
class GameSerializer;

class WaterVertex;

class GameObjectView {
  public:
    GameObjectView( GameObject & obj,
                    const ProtoObject &p );

    GameObjectView( MyGL::Scene & s,
                    World       & wrld,
                    const ProtoObject &p,
                    const PrototypesLoader & pl );


    GameObjectView( MyGL::Scene & s,
                    World       & wrld,
                    ParticleSystemEngine & psysEngine,
                    const ProtoObject &p,
                    const PrototypesLoader & pl );

    ~GameObjectView();

    MyGL::Color teamColor;

    void freePhysic();

    void loadView(const Resource &r, Physics &p, bool env );
    void loadView(const Resource &r, const ProtoObject::View &src, bool isEnv ) ;
    void loadView(const Model &model, const ProtoObject::View &view);
    void loadView(const MyGL::Model<WaterVertex> &model);

    void setViewPosition(float x, float y, float z );
    void setViewPosition(float x, float y, float z,
                         float interp );

    double viewHeight() const;

    void setViewSize(float x, float y, float z);
    void updatePos();

    enum Selection{
      selStd    = 0,
      selOver   = 1,
      selCmd    = 2,
      selMoveTo = 3,
      selAtk    = 4
      };

    void setSelectionVisible( bool v, Selection s );
    void higlight( int time, Selection s );
    void setVisible( bool v );
    void setVisible_perf( bool v );

    void rotate(int delta);
    void setRotation(int delta);
    void setViewDirection( int x, int y );
    void viewDirection( int &x, int &y ) const;
    double rAngle() const;

    void tick();

    int x() const;
    int y() const;

    void setPosition( int x, int y );

    double radius() const;
    double rawRadius() const;

    Physics* physicEngine() const;
    void serialize( GameSerializer &s);

    const ProtoObject &getClass() const;
    MyGL::Matrix4x4 _transform() const;

  private:
    void setForm( EnvObject &obj, const Physics::Sphere &f );
    void setForm( EnvObject &obj, const Physics::Box &f    );
    void setForm(const Physics::AnimatedSphere &f);
    void setForm(const Physics::AnimatedBox &f);

    void setupMaterials( MyGL::AbstractGraphicObject &obj,
                         const ProtoObject::View &src );

    template< class Object >
    void setViewPosition( Object &obj,
                          const ProtoObject::View & v,
                          float x,
                          float y, float z,
                          float interp);

    template< class Object >
    void setViewSize( Object &obj,
                      const ProtoObject::View & v,
                      float x, float y, float z );

    struct M{
      int x, y, z;
      double selectionSize[3], radius;
      double modelSize[3], rndSize[3];

      int intentDirX, intentDirY;
      } m;

    struct {
      Physics::AnimatedSphere sphere;
      Physics::AnimatedBox    box;
      } anim;

    std::vector< EnvObject > env;
    std::vector< MyGL::GraphicObject > view;
    std::vector< std::unique_ptr<SmallGraphicsObject> > smallViews;
    std::vector< ParticleSystem >      particles;

    Physics * physic;

    static const int selectModelsCount = 5;
    std::unique_ptr<MyGL::GraphicObject> selection[selectModelsCount];
    int htime[selectModelsCount];

    MyGL::Scene & scene;
    World       & wrld;
    ParticleSystemEngine & psysEngine;
    const ProtoObject * cls;
    const PrototypesLoader & prototypes;

    template< class Rigid >
    void updatePosRigid( Rigid & r );

    void updatePosRigid( Physics::Sphere & r, size_t i );
    void updatePosRigid( Physics::Box    & r, size_t i );

    template< class Obj >
    void serialize(GameSerializer &s, Obj * obj , bool store);

    void init();
  };

#endif // GAMEOBJECTVIEW_H
