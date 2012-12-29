#include "editterrainpanel.h"

#include <MyWidget/Layout>
#include "button.h"
#include "scroolbar.h"

EditTerrainPanel::EditTerrainPanel(Resource &res) {
  layout().setMargin( 6 );
  setLayout( MyWidget::Vertical );

  Button *btn = new Button(res);
  btn->clicked.bind( *this, &EditTerrainPanel::setupEHeight );
  btn->setText("Edit height");
  layout().add( btn );

  btn = new Button(res);
  btn->clicked.bind( *this, &EditTerrainPanel::setupCombo );
  btn->setText("Edit height");
  layout().add( btn );

  ScroolBar *sb = new ScroolBar(res);
  sb->setOrientation( MyWidget::Horizontal );
  sb->valueChanged.bind( *this, &EditTerrainPanel::setR );
  sb->setRange(0, 20);
  layout().add( sb );

  layout().add( new Widget() );
  }

void EditTerrainPanel::disableEdit(int) {
  m.isEnable = false;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setupEHeight() {
  m.map  = Terrain::EditMode::Up;
  m.wmap = Terrain::EditMode::None;

  m.isEnable = true;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setupCombo() {
  m.map  = Terrain::EditMode::Up;
  m.wmap = Terrain::EditMode::Up;

  m.isEnable = true;
  toogleEditLandMode(m);
  }

void EditTerrainPanel::setR(int r) {
  m.R = r;
  toogleEditLandMode(m);
  }
