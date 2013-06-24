#ifndef DESERTSTRIKESCENARIOWIDGETS_H
#define DESERTSTRIKESCENARIOWIDGETS_H

#include "desertstrikescenario.h"

#include "game.h"
#include "gui/ingamecontrols.h"

class RichText;
class UnitView;

struct DesertStrikeScenario::NumButton: public Button {
  NumButton( Resource & r );
  void paintEvent(Tempest::PaintEvent &e);

  int  num;
  Font font;
  Texture numFrame;
  };

struct DesertStrikeScenario::BuyButton: public NumButton {
  BuyButton( Resource & r,
             const ProtoObject& obj,
             PlInfo & pl,
             int tier );
  void emitClick();
  void paintEvent(Tempest::PaintEvent &e);

  Tempest::signal<const ProtoObject&> onClick;

  const ProtoObject& p;
  PlInfo & pl;
  int tier;
  };

struct DesertStrikeScenario::GradeButton: public Button {
  GradeButton( Resource & r,
               PlInfo & p,
               const std::string& obj,
               const int t );
  void emitClick();
  void paintEvent(Tempest::PaintEvent &e);

  Tempest::signal<int> onClick;
  int type;
  PlInfo & pl;
  Texture texture;
  };

struct DesertStrikeScenario::TranscurentPanel: public Tempest::Widget {
  TranscurentPanel( Resource & res );
  void paintEvent(Tempest::PaintEvent &e);

  Resource  & res;
  Tempest::Bind::UserTexture frame;
  };

struct DesertStrikeScenario::Minimap: MiniMapView{
  struct BuyButton;
  struct GradeButton;

  struct Inf{
    Widget * widget;
    RichText * ledit, *info[2];

    GradeButton * grade;
    };

  Minimap( Resource &res,
           Game & game,
           PlInfo & pl );

  void buildBase( Resource &res, Inf & inf );
  void buildCas( Resource &res, Inf & inf );
  void paintEvent(Tempest::PaintEvent &e);
  void mouseDownEvent(Tempest::MouseEvent &);
  void setupUnit( const std::string & unit );
  void updateValues();
  void hideInfo();
  void buy();
  void grade();
  void sell();

  UnitView* base;

  Inf inf[2];
  int infID;

  std::string unitToBuy;

  Game &game;
  PlInfo & pl;
  };

struct DesertStrikeScenario::SpellPanel: public TranscurentPanel {
  SpellPanel( Resource & res,
              Game & game,
              PlInfo & pl );
  void spell( int i );

  Game & game;
  };

struct DesertStrikeScenario::BuyUnitPanel: public TranscurentPanel {
  BuyUnitPanel( Resource & res,
                Game & game,
                PlInfo & pl,
                Minimap * mmap);
  void onUnit( const ProtoObject & p );
  void setupBuyPanel( const std::string & s );

  template< int w, int h >
  Widget* mkPanel( PlInfo & pl, const char * pr[w][h] ){
    using namespace Tempest;

    Widget *w = new Widget();
    w->setLayout(Vertical);

    for( int i=0; i<3; ++i ){
      Widget* l = new Widget();
      l->setLayout( Horizontal );

      for( int r=0; r<4; ++r ){
        if( pr[i][r] ){
          const ProtoObject & obj = game.prototype(pr[i][r]);

          DesertStrikeScenario::BuyButton * u =
              new DesertStrikeScenario::BuyButton(res, obj, pl, i);
          u->onClick.bind( *this, &BuyUnitPanel::onUnit );
          l->layout().add( u );
          }
        }
      w->layout().add(l);
      }

    return w;
    }

  void setTab( int id );

  Game &game;
  Minimap * mmap;

  Widget* layers[4];
  };

struct DesertStrikeScenario::UpgradePanel: public TranscurentPanel {
  UpgradePanel( Resource & res,
                Game & game,
                PlInfo & pl,
                DesertStrikeScenario::BuyUnitPanel *mmap );

  void buy( const int grade );

  Game & game;
  DesertStrikeScenario::BuyUnitPanel *mmap;
  };


#endif // DESERTSTRIKESCENARIOWIDGETS_H
