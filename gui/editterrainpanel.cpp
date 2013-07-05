#include "editterrainpanel.h"

#include <Tempest/Layout>
#include "button.h"
#include "scroolbar.h"

#include "scroolwidget.h"
#include "prototypesloader.h"

struct EditTerrainPanel::TileBtn: public Button{
  TileBtn( Resource & res,
           const std::string& tile ):Button(res), tile(tile){
    clicked.bind( *this, &TileBtn::beep );
    }

  void beep(){
    clickedEx( tile );
    }

  Tempest::signal< const std::string& > clickedEx;
  std::string tile;
  };

EditTerrainPanel::EditTerrainPanel( Resource &res,
                                    const PrototypesLoader &prototypes ) {
  layout().setMargin( 6 );
  setLayout( Tempest::Vertical );

  Widget *w = 0;
  w = new Widget();
  w->setLayout( Tempest::Horizontal );

  Button *btn = new Button(res);
  btn->clicked.bind( *this, &EditTerrainPanel::setupEHeight );
  btn->setText("Edit height");
  w->layout().add( btn );

  btn = new Button(res);
  btn->clicked.bind( *this, &EditTerrainPanel::setupCombo );
  btn->setText("Edit height");
  w->layout().add( btn );
  w->setMaximumSize( w->sizePolicy().maxSize.w, 27 );
  w->setSizePolicy( Tempest::Preferred, Tempest::FixedMax );
  layout().add( w );

  w = new Widget();
  w->setLayout( Tempest::Horizontal );
  btn = new Button(res);
  btn->clicked.bind( *this, &EditTerrainPanel::setupAlign );
  btn->setText("Align height");
  w->layout().add( btn );

  btn = new Button(res);
  btn->clicked.bind( *this, &EditTerrainPanel::setupSmooth );
  btn->setText("Smooth height");
  w->layout().add( btn );
  w->setMaximumSize( w->sizePolicy().maxSize.w, 27 );
  w->setSizePolicy( Tempest::Preferred, Tempest::FixedMax );
  layout().add( w );

  w = new Widget();
  w->setLayout( Tempest::Horizontal );
  btn = new Button(res);
  btn->clicked.bind( *this, &EditTerrainPanel::setupRm );
  btn->setText("rm objects");
  w->layout().add( btn );

  w->setMaximumSize( w->sizePolicy().maxSize.w, 27 );
  w->setSizePolicy( Tempest::Preferred, Tempest::FixedMax );
  layout().add( w );

  ScroolBar *sb = new ScroolBar(res);
  sb->setOrientation( Tempest::Horizontal );
  sb->valueChanged.bind( *this, &EditTerrainPanel::setR );
  sb->setRange(0, 20);

  layout().add( sb );

  w = new Widget();
  w->setLayout( Tempest::Horizontal );

  btn = new Button(res);
  btn->setText("Main");
  btn->clicked.bind( this, &EditTerrainPanel::setTexID<0> );
  Tempest::SizePolicy policy = btn->sizePolicy();
  policy.typeH = Tempest::Preferred;
  btn->setSizePolicy( policy );
  w->layout().add( btn );

  btn = new Button(res);
  btn->setText("Secondary");
  btn->clicked.bind( this, &EditTerrainPanel::setTexID<1> );
  btn->setSizePolicy( policy );
  w->layout().add( btn );

  policy = w->sizePolicy();
  policy.minSize.h = 30;
  policy.typeV = Tempest::FixedMin;
  //w->setSizePolicy( btn->sizePolicy() );
  w->setSizePolicy( policy );
  w->layout().setMargin(0);

  layout().add( w );

  ScroolWidget *sw = new ScroolWidget(res);
  auto p = prototypes.allClasses();

  for( size_t i=0; i<p.size(); ++i ){
    if( p[i]->isLandTile() ) {
      TileBtn *b = new TileBtn(res, p[i]->name);
      b->setText( p[i]->name );
      b->clickedEx.bind( *this, &EditTerrainPanel::setTexture );

      sw->centralWidget().layout().add( b );
      }
    }

  layout().add( sw );
  }

void EditTerrainPanel::disableEdit(int) {
  m.isEnable = false;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setupEHeight() {
  int  r  = m.R;
  m = Terrain::EditMode();
  m.R = r;
  m.map  = Terrain::EditMode::Up;
  m.wmap = Terrain::EditMode::None;

  m.isEnable = true;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setupCombo() {
  int  r  = m.R;
  m = Terrain::EditMode();
  m.R = r;
  m.map  = Terrain::EditMode::Up;
  m.wmap = Terrain::EditMode::Up;

  m.isEnable = true;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setupAlign() {
  int  r  = m.R;
  m = Terrain::EditMode();
  m.R = r;
  m.map  = Terrain::EditMode::Align;
  m.wmap = Terrain::EditMode::Align;

  m.isEnable = true;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setupSmooth() {
  int  r  = m.R;
  m = Terrain::EditMode();
  m.R = r;
  m.map  = Terrain::EditMode::Smooth;
  m.wmap = Terrain::EditMode::Smooth;

  m.isEnable = true;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setupRm() {
  int  r  = m.R;
  m = Terrain::EditMode();
  m.R = r;
  m.map  = Terrain::EditMode::RemoveObj;
  m.wmap = Terrain::EditMode::RemoveObj;

  m.isEnable = true;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setTexture(const std::string &str) {
  bool mt = m.isSecondaryTexturing;
  int  r  = m.R;
  m = Terrain::EditMode();
  m.R = r;

  m.isSecondaryTexturing = mt;
  m.map  = Terrain::EditMode::None;
  m.wmap = Terrain::EditMode::None;

  m.isEnable = true;
  m.texture  = str;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setR(int r) {
  m.R = r;
  toogleEditLandMode(m);
  }
