#ifndef WORLD_H
#define WORLD_H

#include "landscape/terrain.h"
#include "physics/physics.h"
#include "game/gameobject.h"

#include <MyWidget/Event>

#include <memory>

class BehaviorMSGQueue;
class PrototypesLoader;
class Game;
class Player;

namespace MyWidget{
  class Painter;
  }

class WeakWorldPtr;

class World {
  public:
    typedef std::shared_ptr<GameObject> PGameObject;

    World( GraphicsSystem& g,
           Resource      & r,
           PrototypesLoader &p,
           Game & game,
           int w, int h );

    void clickEvent( int x, int y, const MyWidget::MouseEvent &e );

    static double coordCast( int icoord );
    static double coordCastP( double icoord );
    static int    coordCastD( double dcoord );
    static int    coordCastD( int dcoord );

    void moveCamera( double x, double y );
    void tick();

    MyGL::Camera   camera;
    const MyGL::Scene& getScene() const;

    GameObject& addObject( const std::string & proto,
                           int pl = 0 );

    GameObject& addObjectEnv( const std::string & proto );
    GameObject& addObject( const ProtoObject & p,
                           int pl, bool env );
    void deleteObject( GameObject* obj );

    size_t objectsCount() const;
    GameObject& object( size_t i );
    WeakWorldPtr objectWPtr( size_t i );
    WeakWorldPtr objectWPtr( GameObject* i );
    //std::vector< PGameObject >& selectedObjects();

    const Terrain& terrain() const;
    Terrain& terrain();

    void removeObject( GameObject* obj );
    void updateMouseOverFlag(double x0, double y0, double x1, double y1);
    void updateSelectionFlag( BehaviorMSGQueue & msg, int pl );
    void paintHUD( MyWidget::Painter &, int w, int h );

    Player& player( int id );

    Game & game;

    void addWptr( WeakWorldPtr* );
    void delWptr( WeakWorldPtr* );

    void addWarehouse( GameObject * ptr );
    void delWarehouse( GameObject * ptr );
    const std::vector< GameObject* >& warehouse() const;

    void addResouce( GameObject * ptr );
    void delResouce( GameObject * ptr );
    const std::vector< GameObject* >& resouce() const;

    void setMousePos( int x, int y );
    int mouseX() const;
    int mouseY() const;
  private:

    Physics physics;
    Terrain terr;

    MyGL::Scene scene;
    int mpos[2];

    std::vector< PGameObject > gameObjects, eviObjects;
    // std::vector< PGameObject > selected;
    std::vector< WeakWorldPtr* > wptrs;

    std::vector< GameObject* > warehouses, resouces;

    void deleteObject( std::vector< PGameObject > & c, GameObject* obj );
    void del( std::vector<GameObject*> & x, GameObject *p);

    int tx, ty;

    PGameObject terrainView, waterView;

    GraphicsSystem& graphics;
    Resource      & resource;
    PrototypesLoader &prototypes;

    void initTerrain();
    void createTestMap();

    void createResp( int pl, int x, int y, int minX, int minY );
};

#endif // WORLD_H
