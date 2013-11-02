#ifndef DESERTSTRIKESCENARIOWIDGETS_H
#define DESERTSTRIKESCENARIOWIDGETS_H

#include "desertstrikescenario.h"

#include "game.h"
#include "gui/ingamecontrols.h"
#include "gui/modalwindow.h"

#include <cstdint>

class RichText;
class UnitView;

struct DesertStrikeScenario::NumButton: public Button {
  NumButton( Resource & r );
  void paintEvent(Tempest::PaintEvent &e);

  void gestureEvent(Tempest::AbstractGestureEvent &e);

  int  num;
  Tempest::Font font;
  Tempest::Sprite numFrame;

  Tempest::signal<> sellGesture;
  };

struct DesertStrikeScenario::BuyButton: public NumButton {
  BuyButton( Resource & r,
             const ProtoObject& obj,
             DPlayer & pl,
             int tier );
  void emitClick();
  void emitSell();
  void paintEvent(Tempest::PaintEvent &e);

  Tempest::signal<const ProtoObject&> onClick, onSell;

  const ProtoObject& p;
  DPlayer & pl;
  Tempest::Sprite texture;
  int tier;
  };

struct DesertStrikeScenario::GradeButton: public Button {
  GradeButton( Resource & r,
               DPlayer & p,
               const std::string& obj,
               const int t );
  void emitClick();
  void paintEvent(Tempest::PaintEvent &e);

  Tempest::signal<int> onClick;
  int type;
  DPlayer & pl;
  Tempest::Sprite texture;
  };

struct DesertStrikeScenario::TranscurentPanel: public Tempest::Widget {
  TranscurentPanel( Resource & res );
  void paintEvent(Tempest::PaintEvent &e);

  void mouseDownEvent(Tempest::MouseEvent &e);

  Resource  & res;
  Tempest::Sprite frame;
  };

struct DesertStrikeScenario::Minimap: MiniMapView{
  struct BuyButton;
  struct GradeButton;

  struct Inf{
    Widget * widget;
    RichText * ledit, *info[2][2];

    GradeButton * grade;
    };

  Minimap( Resource &res,
           Game & game,
           DPlayer & pl );

  void buildBase ( Resource &res, Inf & inf );
  void buildCas  ( Resource &res, Inf & inf );
  void buildGrade( Resource &res, Inf & inf );

  void mkInfoPanel(Resource &res, Inf &inf, Widget* owner );

  void paintEvent(Tempest::PaintEvent &e);
  void mouseDownEvent(Tempest::MouseEvent &);
  void setupUnit( const std::string & unit );
  void setGrade();
  void updateValues();
  void hideInfo();
  void buy();
  void grade();
  void sell();

  Tempest::signal<std::string> onUnit, onBuilding;
  UnitView* base;

  Inf inf[3];
  int infID;

  std::string unitToBuy;

  Game &game;
  DPlayer & pl;
  };

struct DesertStrikeScenario::SpellPanel: public TranscurentPanel {
  SpellPanel( Resource & res,
              Game & game,
              DPlayer & pl );
  void spell( int i );
  Tempest::signal<> toogleCameraMode;

  Game & game;

  class SpellButton;
  class CameraButton;

  void setupHook ( const std::string& unit );
  void setupHookU( const std::string& unit );
  };

struct DesertStrikeScenario::BuyUnitPanel: public TranscurentPanel {
  BuyUnitPanel( Resource & res,
                Game & game,
                DPlayer & pl,
                Minimap * mmap);
  void onUnit( const ProtoObject & p );
  void onGrade();
  void setupBuyPanel( const std::string & s );

  void setup( int id, bool e[3][4] );

  struct PanelW{
    Widget *w;
    Button *btns[3][4];
    };
  template< int w, int h >
  PanelW mkPanel( DPlayer & pl, const char * pr[w][h],
                   Button* (BuyUnitPanel::*f)( Resource & ,
                                               const char* ,
                                               DPlayer &,
                                               int ) ){
    using namespace Tempest;
    PanelW pw;

    Widget *w = new Widget();
    w->setLayout(Vertical);
    w->layout().setSpacing(0);

    for( int i=0; i<3; ++i ){
      Widget* l = new Widget();
      l->layout().setSpacing(0);
      l->setLayout( Horizontal );

      for( int r=0; r<4; ++r ){
        pw.btns[i][r] = 0;
        if( pr[i][r] ){
          pw.btns[i][r] = (this->*f)(res, pr[i][r], pl, i);
          l->layout().add( pw.btns[i][r] );
          }
        }
      w->layout().add(l);
      }

    pw.w = w;
    return pw;
    }

  Button* mkBuyCasBtn( Resource & r,
                        const char* obj,
                        DPlayer & pl,
                        int tier );
  Button* mkBuyUnitBtn( Resource & r,
                        const char* obj,
                        DPlayer & pl,
                        int tier );
  Button* mkBuyGradeBtn( Resource & r,
                         const char* obj,
                         DPlayer & pl,
                         int tier );

  void setTab( int id );

  Game &game;
  Minimap * mmap;

  PanelW layers[4];
  };

struct DesertStrikeScenario::UpgradePanel: public TranscurentPanel {
  UpgradePanel( Resource & res,
                Game & game,
                DPlayer & pl,
                DesertStrikeScenario::BuyUnitPanel *mmap );

  void buy( const int grade );

  Game & game;
  DesertStrikeScenario::BuyUnitPanel *mmap;

  Tempest::signal<int> onPage;
  };

struct DesertStrikeScenario::MiniBuyPanel: public TranscurentPanel {
  MiniBuyPanel( Resource & res,
                Game & game,
                DPlayer & pl );

  void buyU( const ProtoObject & unitToBuy );
  void buyG( const ProtoObject & gradeToBuy );

  void sellU( const ProtoObject & unitToBuy );
  Game &game;
  };

class DesertStrikeScenario::CentralPanel: public Tempest::Widget {
  public:
    CentralPanel( DesertStrikeScenario &ds, Resource & res );

    void setRemTime(int t);
    void setColor( const Tempest::Color & c );
  protected:
    void paintEvent(Tempest::PaintEvent &e);

  private:
    Resource & res;
    DesertStrikeScenario &ds;
    Tempest::Sprite bg, cride;

    int time;
    Tempest::Color cl;
  };

struct DesertStrikeScenario::WinLoseScreen: public ModalWindow {
  WinLoseScreen(Resource& res, Tempest::Widget *owner , Game &game);

  void paintEvent(Tempest::PaintEvent &e);

  bool isWin;
  uint64_t stime;
  };

struct DesertStrikeScenario::Hint: public ModalWindow{
  Hint(Resource& res, Tempest::Widget *owner, Game &game );
  ~Hint();

  void paintEvent(Tempest::PaintEvent &e);

  Tempest::Texture2d hintView;
  Game &game;
  };

struct DesertStrikeScenario::UInfo: public ModalWindow{
  UInfo(Resource& res, Tempest::Widget *owner, Game &game );
  ~UInfo();

  void paintEvent(Tempest::PaintEvent &e);

  Tempest::Texture2d hintView;
  Game &game;
  };

#endif // DESERTSTRIKESCENARIOWIDGETS_H
