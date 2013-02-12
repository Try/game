#ifndef GAME_H
#define GAME_H

#include "graphics/graphicssystem.h"
#include "resource.h"
#include <MyGL/Camera>

#include <MyWidget/Event>

#include "game/world.h"
#include "game/player.h"
#include "prototypesloader.h"
#include <memory>

#include "gui/maingui.h"
#include "behavior/behaviormsgqueue.h"
#include <MyWidget/Event>
#include "util/fileserialize.h"
#include "gui/inputhook.h"

#include "network/netuser.h"
#include "sound/sound.h"

#include "game/scenario.h"

class GameSerializer;
class Scenario;

class Game {
  public:
    Game( void* hwnd, int w, int h, bool isFullScreen );

    void tick();
    void render(size_t dt);
    void resizeEvent( int w, int h );

    void mouseDownEvent( MyWidget::MouseEvent &e );
    void mouseUpEvent   ( MyWidget::MouseEvent &e );

    void mouseMoveEvent ( MyWidget::MouseEvent &e );

    void mouseWheelEvent ( MyWidget::MouseEvent &e );

    void scutEvent   ( MyWidget::KeyEvent &e );
    void keyDownEvent( MyWidget::KeyEvent &e );
    void keyUpEvent  ( MyWidget::KeyEvent &e );

    MyWidget::signal<bool> toogleFullScreen;
    void toogleFullScr();

    Player & player( int i );
    Player & player();
    size_t plCount() const;

    void setPlaylersCount( int c );
    void addEditorObject(const std::string &p, int pl,
                          int x, int y , int rAngle, size_t unitPl);
    void moveEditorObject( int pl, int x, int y );
    void rotateEditorObject( int pl, int x );
    void nextEditorObject( int pl );
    void delEditorObject( int pl );

    bool message( int pl,
                  AbstractBehavior::Message m,
                  int x,
                  int y,
                  AbstractBehavior::Modifers md = BehaviorEvent::NoModifer );

    bool message( int pl,
                  AbstractBehavior::Message m,
                  int x,
                  int y,
                  const std::string & spell,
                  AbstractBehavior::Modifers md = BehaviorEvent::NoModifer );

    bool message( int pl,
                  AbstractBehavior::Message m,
                  size_t id,
                  AbstractBehavior::Modifers md = BehaviorEvent::NoModifer );

    bool message( int pl,
                  AbstractBehavior::Message m,
                  size_t id,
                  const std::string & spell,
                  AbstractBehavior::Modifers md = BehaviorEvent::NoModifer );

    bool instalHook( InputHookBase* b );
    void removeHook( InputHookBase* b );

    const ProtoObject& prototype( const std::string& s ) const;
    Resource &resources();
    const PrototypesLoader & prototypes() const;

    void setupMaterials( MyGL::AbstractGraphicObject &obj,
                         const ProtoObject::View &src,
                         const MyGL::Color &teamColor );

    MyGL::Matrix4x4& shadowMat();

    void save(const std::wstring &str);
    void load(const std::wstring &str);

    bool isFullScr() const;

    void setCurrectPlayer( int pl );
    void setupAsServer();
    void setupAsClient(const std::wstring &str);

    void onUnitRemove( size_t i );

    void pause( bool p );
    void unsetPause();
    bool isPaused() const;

    void setCameraPos( GameObject& obj );
    void minimapEvent( float x, float y,
                       MyWidget::Event::MouseButton b,
                       MiniMapView::Mode m);

    Scenario& scenario();
    MyWidget::signal<> updateMissionTargets;

    MyWidget::signal<> exitGame;
  private:
    void* hwnd;
    bool  paused;
    bool isFullScreen;

    bool acceptMouseObj;

    GraphicsSystem graphics;
    SoundDevice    soundDev;

    Resource         resource;
    PrototypesLoader proto;

    MainGui  gui;
    BehaviorMSGQueue msg;
    FileSerialize serializator;

    void addPlayer();
    int currentPlayer;
    void createEditorObject( const ProtoObject & p, int pl );
    std::vector< std::unique_ptr<Player> > players;
    std::vector< std::unique_ptr<World> >  worlds;
    World * world;

    //double spinX, spinY;
    int w, h;
    bool mouseTracking;
    int  selectionRectTracking;

    MyWidget::Point lastMPos,curMPos;
    MyWidget::KeyEvent::KeyType   lastKEvent;

    struct F3{
      float data[3];
      };
    F3 unProject( int x, int y, float destZ );
    F3 unProject( int x, int y );
    F3 project( float x, float y, float z );

    void moveCamera();

    struct Fps{
      int n, time;
      } fps;

    int     sendDelay;
    std::unique_ptr<NetUser>  netUser;
    std::unique_ptr<Scenario> mscenario;

    void serialize( GameSerializer &s);

    void log(const std::string &l );

    void onUnitsSelected( std::vector<GameObject*>& u, Player &pl);
    void onUnitDied( GameObject& u, Player &pl);

    void onRender(double dt);

  friend class World;
  };

#endif // GAME_H
