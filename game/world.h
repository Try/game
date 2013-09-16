#ifndef WORLD_H
#define WORLD_H

#include "landscape/terrain.h"
#include "physics/physics.h"
#include "game/gameobject.h"
#include "graphics/particlesystemengine.h"
#include "graphics/hudanim.h"

#include "spatialindex.h"
#include "algo/wayfindrequest.h"

#include <Tempest/Event>

#include <memory>
#include <unordered_set>

#include "threads/async.h"
#include "threads/mutex.h"

class BehaviorMSGQueue;
class PrototypesLoader;
class Game;
class Player;
class GameSerializer;

namespace Tempest{
  class Painter;
  }

class WeakWorldPtr;

class World {
  public:
    typedef std::shared_ptr<GameObject> PGameObject;

    World( Game & game,
           int w, int h );
    ~World();

    Tempest::signal< AbstractGraphicObject &,
                      const ProtoObject::View &,
                      const Tempest::Color & > setupMaterial;

    void clickEvent( int x, int y, const Tempest::MouseEvent &e );
    void onRender(double dt);

    static inline double coordCast(int icoord) {
      return icoord/4048.f;
      }

    static inline double coordCastP(double icoord) {
      return icoord/4048.f;
      }

    static inline int coordCastD(double dcoord) {
      return int(dcoord*4048.0);
      }

    static inline int coordCastD(int dcoord) {
      return dcoord*4048;
      }

    void moveCamera( double x, double y, bool angle = true );
    void tick();

    Tempest::Camera   camera;
    Scene &getScene();
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

    void paintHUD( Tempest::Painter &, int w, int h );

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

      bool operator == ( const CameraViewBounds& b ) const;
      bool operator != ( const CameraViewBounds& b ) const;
      };

    const CameraViewBounds& cameraBounds() const;
    void setCameraBounds( const CameraViewBounds& c );

    void wayFind( int x, int y, GameObject* obj );

    void updateIntent( GameObjectView* v );
    void clrUpdateIntents();

    void onObjectMoved( GameObject* obj,
                        int x, int y,
                        int nx, int ny );
    void onObjectDelete( GameObject* obj );
  private:
    bool    isRunning;

    Scene scene;
    std::unique_ptr<Terrain> terr;

    int mpos[3];
    Terrain::EditMode editLandMode;
    CameraViewBounds cameraVBounds;
    struct IntCameraBounds{
      int rx, ry, lx, ly;
      } lcamBds;

    std::vector< PGameObject > gameObjects, eviObjects;
    std::vector< PGameObject > nonBackground;
    std::unordered_set<GameObjectView*, std::hash<void*> > updatePosIntents;

    std::shared_ptr<SpatialIndex> spatialId;
    // std::vector< PGameObject > selected;
    std::vector< WeakWorldPtr* > wptrs;
    GameObject* mouseObject;

    std::vector< std::shared_ptr<EfectBase> > hudAnims;

    std::vector< GameObject* > warehouses, resouces;

    void deleteObject( std::vector< PGameObject > & c, GameObject* obj );
    void del( std::vector<GameObject*> & x, GameObject *p);

    int tx, ty;

    // PGameObject terrainView, waterView;

    Graphics      & graphics;
    Resource      & resource;
    const PrototypesLoader &prototypes;

    ParticleSystemEngine particles;

    std::unique_ptr<WayFindRequest> wayFindRq;

    void initTerrain();
    void createTestMap();

    void computePhysic(void *);
    Future physicCompute;

    Tempest::Sprite green, bar, gray;

    void createResp( int pl, int x, int y, int minX, int minY );

    bool isUnitUnderMouse(Tempest::Matrix4x4 & gmMat, const GameObject &obj,
                           int mx, int my, int w, int h , int &dist) const;

    Tempest::Rect projectUnit(GameObject& obj , const Tempest::Matrix4x4 &gmMat, int w, int h);
};

#endif // WORLD_H
