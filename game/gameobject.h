#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <memory>
#include <unordered_set>

class Scene;

#include "graphics/graphicssystem.h"
#include "physics/physics.h"
#include "behavior/behavior.h"
#include "game/envobject.h"
#include "game/protoobject.h"
#include "game/gameobjectview.h"
#include "graphics/hudanim.h"

#include <Tempest/signal>

class WaterVertex;
class Resource;
class ProtoObject;
class PrototypesLoader;
class World;
class Player;
class GameSerializer;
class Spell;

class ObjectEfect;

class Bullet;

class GameObject {
  public:
    GameObject( Scene & s,
                World & wrld,
                const ProtoObject &p,
                const PrototypesLoader & pl );
    ~GameObject();
    Tempest::signal<GameObject&> onDied;

    std::vector<GameObject*> colisions;

    std::wstring hint;
    int envLifeTime;

    int distanceSQ( const GameObject& other ) const;
    int distanceSQ( int x, int y ) const;
    int distanceQ ( int x, int y ) const;
    int distanceQL( int x, int y ) const;

    void loadView(const Resource &r, Physics &p,
                   bool env );
    void loadView(const Model &model , const ProtoObject::View &pview);

    void loadView(const Model &model );

    void setPosition( int x, int y );
    void setPosition( int x, int y, int z );
    void setPositionSmooth( int x, int y, int z );
    void syncView(double dt);

    void setViewSize( float x, float y, float z );
    void setViewSize( float s );
    static void setViewSize( GraphicObject& obj,
                             const ProtoObject::View &v,
                             float x, float y, float z);

    void updatePos();
    void tick( const Terrain & terrain );
    void tickMv( const Terrain & terrain );

    const ProtoObject & getClass() const;
    void setClass( const ProtoObject* p );

    int x() const;
    int y() const;
    int z() const;

    float viewX() const;
    float viewY() const;
    float viewZ() const;

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

    Tempest::Matrix4x4 _transform() const;
    Behavior behavior;

    bool isOnMove() const;
    bool isRepositionMove() const;

    World& world();
    Game&  game();
    Scene& getScene();

    void setPlayer( int pl );
    int playerNum() const;
    Player& player();

    int team() const;

    bool hasHostCtrl() const;
    bool isMoviable() const;
    bool isMineralMove() const;

    double viewHeight() const;

    const Tempest::Color& teamColor() const;
    void setTeamColor( const Tempest::Color& cl );

    static void setViewPosition( GraphicObject& obj,
                                 const ProtoObject::View &v,
                                 float x, float y, float z );

    void setHP( int h);
    int  hp() const;

    std::shared_ptr<Bullet> reciveBulldet( const std::string& view );

    void serialize( GameSerializer &s );
    void setColisionDisp( int dx, int dy );
    void incColisionDisp( int dx, int dy );

    void higlight(int time, GameObjectView::Selection type );
    bool isVisible( const Frustum & f ) const;

    void setCoolDown( size_t spellID, int v );
    int  coolDown   ( size_t spellID ) const;

    void applyForce( float x, float y, float z );
    void applyBulletForce(const GameObject &src);

    void incDieVec( float x, float y, float z );

    GameObjectView &getView();

    void addEfect( ObjectEfect * e );
    void addEfectFlg( EfectBase::UnitEfect e );
    void delEfectFlg( EfectBase::UnitEfect e );
    bool hasEfect( EfectBase::UnitEfect e ) const;
  private:
    GameObject( const GameObject& obj ) = delete;
    GameObject& operator = ( const GameObject& obj ) = delete;

    void setViewPosition(float x, float y, float z , float s);

    void loadView( Resource & r, const ProtoObject::View &src,
                   bool isEnv );

    Scene       & scene;
    World       & wrld;
    const PrototypesLoader & prototypes;

    GameObjectView view;

    const ProtoObject * myClass;
    //Physics * physic;

    struct M{
      int x, y, z;
      int pl;
      int hp;
      bool isEnv;

      bool isSelected, isMouseOwer;
      float dieVec[3];
      } m;

    Behavior::Closure bclos;
    std::vector< std::shared_ptr<Bullet> >     bullets;
    std::vector< std::unique_ptr<ObjectEfect>> efects;
    std::vector< const Spell* > autoCastSpell;

    std::unordered_map<size_t, int> coolDowns;
    std::unordered_set<EfectBase::UnitEfect, std::hash<size_t> > efectFlg;

    void setupMaterials( AbstractGraphicObject &obj,
                         const ProtoObject::View &src );

    friend class GameObjectView;
    friend class UnitView;
  };

#endif // GAMEOBJECT_H
