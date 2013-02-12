#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <memory>

namespace MyGL{
  class GraphicObject;
  class Scene;
  }

#include "graphics/graphicssystem.h"
#include "physics/physics.h"
#include "behavior/behavior.h"
#include "game/envobject.h"
#include "game/protoobject.h"
#include "game/gameobjectview.h"

#include <MyWidget/signal>

class WaterVertex;
class Resource;
class ProtoObject;
class PrototypesLoader;
class World;
class Player;
class GameSerializer;

class Bullet;

class GameObject {
  public:
    GameObject( MyGL::Scene & s,
                World       & wrld,
                const ProtoObject &p,
                const PrototypesLoader & pl );
    ~GameObject();
    MyWidget::signal<GameObject&> onDied;

    std::vector<GameObject*> colisions;

    std::wstring hint;

    int distanceSQ( const GameObject& other ) const;
    int distanceSQ( int x, int y ) const;
    int distanceQ ( int x, int y ) const;
    int distanceQL( int x, int y ) const;

    void loadView(const Resource &r, Physics &p,
                   bool env );
    void loadView(const Model &model , const ProtoObject::View &pview);

    void loadView( const MyGL::Model<WaterVertex> & model );

    void setPosition( int x, int y, int z );
    void setPositionSmooth( int x, int y, int z );
    void syncView(double dt);

    void setViewSize( float x, float y, float z );
    void setViewSize( float s );
    static void setViewSize( MyGL::GraphicObject& obj,
                             const ProtoObject::View &v,
                             float x, float y, float z);

    void updatePos();
    void tick( const Terrain & terrain );
    void tickMv( const Terrain & terrain );

    const ProtoObject & getClass() const;

    int x() const;
    int y() const;
    int z() const;

    bool isSelected() const;
    void select();
    void unSelect();
    void updateSelection();

    void setViewDirection( int lx, int ly );
    void viewDirection( int &x, int &y );
    void rotate( int delta );
    double rAngle() const;

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
    void setTeamColor( const MyGL::Color& cl );

    static void setViewPosition( MyGL::GraphicObject& obj,
                                 const ProtoObject::View &v,
                                 float x, float y, float z );

    void setHP( int h);
    int  hp() const;

    std::shared_ptr<Bullet> reciveBulldet( const std::string& view );

    void serialize( GameSerializer &s );
    void setColisionDisp( int dx, int dy );
    void incColisionDisp( int dx, int dy );

    void higlight(int time, GameObjectView::Selection type );
    void setVisible_perf( bool v );
    bool isVisible_perf() const;

    void setCoolDown( size_t spellID, int v );
    int  coolDown   ( size_t spellID ) const;

    void applyForce( float x, float y, float z );
  private:
    GameObject( const GameObject& obj ) = delete;
    GameObject& operator = ( const GameObject& obj ) = delete;

    void setViewPosition(float x, float y, float z , float s);

    void loadView( Resource & r, const ProtoObject::View &src,
                   bool isEnv );

    MyGL::Scene & scene;
    World       & wrld;
    const PrototypesLoader & prototypes;

    GameObjectView view;

    const ProtoObject * myClass;
    //Physics * physic;

    struct M{
      int x, y, z;
      int pl;
      int hp;

      bool isSelected, isMouseOwer;
      bool isVisible_perf;
      } m;

    Behavior::Closure bclos;
    std::vector< std::shared_ptr<Bullet> > bullets;

    std::unordered_map<size_t, int> coolDowns;

    void setupMaterials( MyGL::AbstractGraphicObject &obj,
                         const ProtoObject::View &src );

    friend class GameObjectView;
    friend class UnitView;
  };

#endif // GAMEOBJECT_H
