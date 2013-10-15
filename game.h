#ifndef GAME_H
#define GAME_H

#include "graphics/graphicssystem.h"
#include "resource.h"
#include <Tempest/Camera>

#include <Tempest/Event>

#include "game/world.h"
#include "game/player.h"
#include "prototypesloader.h"
#include <memory>

#include "gui/maingui.h"
#include "behavior/behaviormsgqueue.h"
#include <Tempest/Event>
#include "util/fileserialize.h"
#include "gui/inputhook.h"
#include "gui/loadscreen.h"

#include "network/netuser.h"
#include "sound/sound.h"

#include "game/scenario.h"

#include <Tempest/Window>
#include <Tempest/Timer>

class GameSerializer;
class Scenario;

class Game : public Tempest::Window {
  public:
    Game( Tempest::Window::ShowMode s );
    ~Game();

    void showMainMenu();
    void tick();
    //void render(size_t dt);
    void render();
    void resizeEvent( Tempest::SizeEvent& );

    void mouseDownEvent ( Tempest::MouseEvent &e );
    void mouseUpEvent   ( Tempest::MouseEvent &e );
    void mouseMoveEvent ( Tempest::MouseEvent &e );
    void mouseWheelEvent ( Tempest::MouseEvent &e );

    void shortcutEvent( Tempest::KeyEvent &e );
    void keyDownEvent( Tempest::KeyEvent &e );
    void keyUpEvent  ( Tempest::KeyEvent &e );

    void closeEvent(Tempest::CloseEvent &e);

    Tempest::signal<bool> toogleFullScreen;
    void toogleFullScr();

    Player & player( int i );
    Player & player();
    int plCount() const;

    void setPlaylersCount( int c );
    void addEditorObject( const std::string &p, int pl,
                          int x, int y, int rAngle, size_t unitPl);
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

    template< class T >
    void message( const T & t ){
      msg.message(t);
      }

    bool instalHook( InputHookBase* b );
    void removeHook( InputHookBase* b );

    const ProtoObject& prototype( const std::string& s ) const;
    const Upgrade&     upgrade  ( const std::string& s ) const;

    Resource &resources();
    const PrototypesLoader & prototypes() const;
    PrototypesLoader & prototypes();

    void setupMaterials( AbstractGraphicObject &obj,
                         const ProtoObject::View &src,
                         const Tempest::Color &teamColor );

    void saveGame();
    void loadGame();
    void save(const std::wstring &str);
    bool load(const std::wstring &str);
    bool loadMission(const std::string  &str);

    void setCurrectPlayer( int pl );
    void setupAsServer();
    void setupAsClient(const std::wstring &str);

    void onUnitRemove( size_t i );

    void pause( bool p );
    void unsetPause();
    bool isPaused() const;

    void setCameraPosition(float x, float y);
    void setCameraPos(GameObject& obj);
    void setCameraPosSmooth(GameObject& obj, float maxK);
    void minimapEvent( float x, float y,
                       Tempest::Event::MouseButton b,
                       MiniMapView::Mode m);

    Scenario& scenario();
    const Scenario& scenario() const;
    Tempest::signal<> updateMissionTargets;

    Tempest::signal<> exitGame;

    World& curWorld();
    bool isReplayMode() const;

    static const int ticksPerSecond;

    template< class T >
    void setupScenario(){
      setScenario( new T(*this, gui, msg) );
      mscenario->onStartGame();
      }
  private:
    bool  paused;//, needToUpdate;
    //bool isFullScreen;

    //bool acceptMouseObj;
    void initGame();

    Graphics graphics;
    SoundDevice    soundDev;

    Resource         resource;
    PrototypesLoader proto;

    MainGui  gui;
    BehaviorMSGQueue msg;
    FileSerialize serializator;

    Tempest::Timer timer;

    std::unique_ptr<LoadScreen>  sload;

    void addPlayer();
    void cancelEdit( int );

    void loadPngWorld(const Tempest::Pixmap &png );

    int currentPlayer;
    void createEditorObject( const ProtoObject & p, int pl );
    void setEditorObjectPl( int pl );

    std::vector< std::shared_ptr<World> >  worlds;
    World * world;

    struct Fps{
      int n, time;
      } fps;

    size_t  updateTime;
    int     sendDelay;
    std::unique_ptr<NetUser>  netUser;
    std::unique_ptr<Scenario> mscenario;

    bool isLoading;
    void serialize( GameSerializer &s);
    void serializeScenario( GameSerializer &s );

    void log(const std::string &l );

    void onUnitsSelected( std::vector<GameObject*>& u, Player &pl);
    void onUnitDied( GameObject& u, Player &pl);

    void onRender(double dt);

    void settingsChanged( const GraphicsSettingsWidget::Settings &s );

    void loadData();

    void setScenario( Scenario * s );

    void computePhysic(void *);
    Future physicCompute;
    bool   isRunning, physicStarted;

    void update();
    void updateOrientation();

  friend class World;
  friend class Scenario;
  };

#endif // GAME_H
