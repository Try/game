#ifndef WORLD_H
#define WORLD_H

#include "landscape/terrain.h"
#include "physics/physics.h"
#include "game/gameobject.h"
#include "graphics/particlesystemengine.h"
#include "graphics/hudanim.h"

#include "spatialindex.h"

#include <MyWidget/Event>

#include <memory>

class BehaviorMSGQueue;
class PrototypesLoader;
class Game;
class Player;
class GameSerializer;

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

    MyWidget::signal< MyGL::AbstractGraphicObject &,
                      const ProtoObject::View &,
                      const MyGL::Color & > setupMaterial;

    void clickEvent( int x, int y, const MyWidget::MouseEvent &e );
    void onRender();

    static double coordCast( int icoord );
    static double coordCastP( double icoord );
    static int    coordCastD( double dcoord );
    static int    coordCastD( int dcoord );

    void moveCamera( double x, double y );
    void tick();

    MyGL::Camera   camera;
    const MyGL::Scene& getScene() const;
    ParticleSystemEngine & getParticles();

    GameObject& addObject( const std::string & proto,
                           int pl = 0 );

    GameObject& addObjectEnv( const std::string & proto );
    GameObject& addObject( const ProtoObject & p,
                           int pl, bool env );
    void deleteObject( GameObject* obj );

    const std::vector< PGameObject >& activeObjects();

    size_t objectsCount() const;
    GameObject& object( size_t i );
    const GameObject& object( size_t i ) const;
    WeakWorldPtr objectWPtr( size_t i );
    WeakWorldPtr objectWPtr( GameObject* i );
    //std::vector< PGameObject >& selectedObjects();

    const Terrain& terrain() const;
    Terrain& terrain();

    void updateMouseOverFlag(double x0, double y0, double x1, double y1);
    void updateSelectionFlag( BehaviorMSGQueue & msg, int pl );
    void updateSelectionClick(BehaviorMSGQueue & msg, int pl,
                              int mx, int my, int w, int h );
    size_t unitUnderMouse(int mx, int my, int w, int h ) const;

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

    void setMousePos( int x, int y, int z );
    void setMouseObject( size_t i );

    int mouseX() const;
    int mouseY() const;
    int mouseZ() const;
    GameObject* mouseObj();

    void toogleEditLandMode( const Terrain::EditMode & m );
    Physics physics;

    void serialize( GameSerializer &s);

    const SpatialIndex& spatial() const;
    void emitHudAnim( const std::string& s,
                      float x, float y, float z );

    float zAt( float x, float y ) const;

    struct CameraViewBounds{
      int x[4];
      int y[4];
      };

    const CameraViewBounds& cameraBounds() const;
    void setCameraBounds( const CameraViewBounds& c );

  private:
    MyGL::Scene scene;
    std::unique_ptr<Terrain> terr;

    int mpos[3];
    Terrain::EditMode editLandMode;
    CameraViewBounds cameraVBounds;
    struct IntCameraBounds{
      int rx, ry, lx, ly;
      } lcamBds;

    std::vector< PGameObject > gameObjects, eviObjects;
    std::vector< PGameObject > nonBackground;
    SpatialIndex spatialId;
    // std::vector< PGameObject > selected;
    std::vector< WeakWorldPtr* > wptrs;
    GameObject* mouseObject;

    std::vector< std::unique_ptr<HudAnim> > hudAnims;

    std::vector< GameObject* > warehouses, resouces;

    void deleteObject( std::vector< PGameObject > & c, GameObject* obj );
    void del( std::vector<GameObject*> & x, GameObject *p);

    int tx, ty;

    // PGameObject terrainView, waterView;

    GraphicsSystem& graphics;
    Resource      & resource;
    PrototypesLoader &prototypes;

    ParticleSystemEngine particles;

    void initTerrain();
    void createTestMap();

    void createResp( int pl, int x, int y, int minX, int minY );

    bool isUnitUnderMouse(MyGL::Matrix4x4 & gmMat, const GameObject &obj,
                           int mx, int my, int w, int h , int &dist) const;
};

#endif // WORLD_H
