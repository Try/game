#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>

namespace MyGL{
  class GraphicObject;
  class Scene;
  }

#include "graphics/graphicssystem.h"
#include "physics/physics.h"
#include "behavior/behavior.h"
#include "game/envobject.h"
#include "game/protoobject.h"

class Resource;
class ProtoObject;
class PrototypesLoader;
class World;
class Player;

class GameObject {
  public:
    GameObject( MyGL::Scene & s,
                World       & wrld,
                const ProtoObject &p,
                const PrototypesLoader & pl );
    ~GameObject();

    int distanceSQ( const GameObject& other ) const;
    int distanceSQ( int x, int y ) const;
    int distanceQ ( int x, int y ) const;
    int distanceQL( int x, int y ) const;

    void loadView( Resource & r, Physics &p,
                   bool env );
    void loadView( const MyGL::Model<> & model );

    void loadView( const MyGL::Model<Terrain::WVertex> & model );

    void setPosition( int x, int y, int z );

    void setViewSize( float x, float y, float z );
    void setViewSize( float s );
    static void setViewSize( MyGL::GraphicObject& obj,
                             const ProtoObject::View &v,
                             float x, float y, float z);

    void updatePos();
    void tick( const Terrain & terrain );

    const ProtoObject & getClass() const;

    int x() const;
    int y() const;
    int z() const;

    bool isSelected() const;
    void select();
    void unSelect();
    void updateSelection();

    void setViewDirection( int lx, int ly );
    void rotate( int delta );

    bool isMouseOwer() const;
    void setMouseOverFlag( bool f );

    double radius() const;
    double rawRadius() const;

    MyGL::Matrix4x4 _transform() const;
    Behavior behavior;

    bool isOnMove() const;
    bool isRepositionMove() const;

    World& world();
    Game&  game();
    MyGL::Scene& getScene();

    void setPlayer( int pl );
    int playerNum() const;
    Player& player();

    int team() const;

    bool hasHostCtrl() const;
    bool isMoviable() const;
    bool isMineralMove() const;

    double viewHeight() const;

    const MyGL::Color& teamColor() const;

    static void setViewPosition( MyGL::GraphicObject& obj,
                                 const ProtoObject::View &v,
                                 float x, float y, float z );

    void setHP( int h);
    int  hp() const;
  private:
    GameObject( const GameObject& obj ) = delete;
    GameObject& operator = ( const GameObject& obj ) = delete;

    void setViewPosition( float x, float y, float z );

    void setForm( const Physics::Sphere & form );
    void setForm( const Physics::Box    & form );
    void setForm( const Physics::AnimatedSphere & form );
    void setForm( const Physics::AnimatedBox & form );

    void loadView( Resource & r, const ProtoObject::View &src,
                   bool isEnv );

    MyGL::Scene & scene;
    World       & wrld;
    const PrototypesLoader & prototypes;

    struct {
      Physics::Sphere sphere;
      Physics::Box    box;
      } form;

    struct {
      Physics::AnimatedSphere sphere;
      Physics::AnimatedBox    box;
      } anim;

    std::vector<EnvObject> env;
    std::vector<MyGL::GraphicObject> view;
    MyGL::GraphicObject selection;

    const ProtoObject * myClass;
    Physics * physic;

    struct M{
      int x, y, z;
      int pl;

      int hp;

      int intentDirX, intentDirY;
      bool isSelected, isMouseOwer;
      double selectionSize[3], radius;

      double modelSize[3];
      MyGL::Color teamColor;
      } m;

    Behavior::Closure bclos;

    void freePhysic();

    void setupMaterials( MyGL::AbstractGraphicObject &obj,
                         const ProtoObject::View &src );

    template< class Rigid >
    void updatePosRigid( Rigid & r );

    void updatePosRigid( Physics::Sphere & r, size_t i );
    void updatePosRigid( Physics::Box    & r, size_t i );
  };

#endif // GAMEOBJECT_H
