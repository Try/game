#include "graphicssettingswidget.h"

#include "listbox.h"
#include "lineedit.h"
#include "checkbox.h"

#include <Tempest/Layout>

#ifdef __ANDROID__
GraphicsSettingsWidget::Settings::API GraphicsSettingsWidget::Settings::api
  = GraphicsSettingsWidget::Settings::openGL;
#else
GraphicsSettingsWidget::Settings::API GraphicsSettingsWidget::Settings::api
  = GraphicsSettingsWidget::Settings::directX;
#endif

GraphicsSettingsWidget::Settings::Settings() {
  shadowMapRes  = 1024;
  bloom         = Hight;
  glow          = true;
  normalMap     = true;

  shadowFilterQ  = 2;
  oclusion       = true;
  shadowTextures = true;
#ifdef __ANDROID__
  shadowMapRes  = 512;
  bloom         = Off;
  glow          = false;
  normalMap     = true;

  shadowFilterQ  = 0;
  oclusion       = false;
  shadowTextures = false;
#endif
  }

GraphicsSettingsWidget::GraphicsSettingsWidget(Resource &res) : Panel(res){
  layout().setMargin(8);

#ifndef __ANDROID__
  resize( 200, 250 );
#else
  resize( 200, 320 );
#endif
  setLayout( Tempest::Vertical );
  setDragable(1);

  ListBox *b = new ListBox(res);
  std::vector< std::wstring > l;
  l.push_back(L"no sm");
  l.push_back(L"sm 512x512");
  l.push_back(L"sm 1024x1024");
  l.push_back(L"sm 2048x2048");
  //l.push_back(L"sm 4096x4096");

  b->setItemList(l);
  b->setCurrentItem(0);
  if( s.shadowMapRes==512 )
    b->setCurrentItem(1);
  if( s.shadowMapRes==1024 )
    b->setCurrentItem(2);
  if( s.shadowMapRes==2048 )
    b->setCurrentItem(3);

  b->onItemSelected.bind( this, &GraphicsSettingsWidget::shadowMapRes);
  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"no sm filter");
  l.push_back(L"4x sm filter");
  l.push_back(L"8x sm filter");

  b->setItemList(l);
  b->setCurrentItem( s.shadowFilterQ );
  b->onItemSelected.bind( this, &GraphicsSettingsWidget::smFilterQ );
  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"bloom off");
  l.push_back(L"bloom low");
  l.push_back(L"bloom hight");

  b->setItemList(l);
  b->setCurrentItem( s.bloom );
  b->onItemSelected.bind( this, &GraphicsSettingsWidget::bloomQ );

  layout().add(b);

  CheckBox *ck = new CheckBox(res);
  ck->setText("glow");
  ck->setChecked( s.glow );
  ck->checked.bind( this, &GraphicsSettingsWidget::glow );
  layout().add(ck);

  ck = new CheckBox(res);
  ck->setText("bumpMap");
  ck->setChecked( s.normalMap );
  ck->checked.bind( this, &GraphicsSettingsWidget::normalMap );
  layout().add(ck);

  ck = new CheckBox(res);
  ck->setText("oclusion");
  ck->setChecked( s.oclusion );
  ck->checked.bind( this, &GraphicsSettingsWidget::oclusion );
  layout().add(ck);

  ck = new CheckBox(res);
  ck->setText("shTexture");
  ck->setChecked( s.shadowTextures );
  ck->checked.bind( this, &GraphicsSettingsWidget::shTexture );
  layout().add(ck);
  }

void GraphicsSettingsWidget::shadowMapRes(int v) {
  int r[] = {0, 512, 1024, 2048, 4096};
  s.shadowMapRes = r[v];

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::smFilterQ(int v) {
  s.shadowFilterQ = v;
  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::bloomQ(int v) {
  s.bloom = Settings::Bloom(v);

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::glow(bool v) {
  s.glow = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::normalMap(bool v) {
  s.normalMap = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::oclusion(bool v) {
  s.oclusion = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::shTexture(bool v) {
  s.shadowTextures = v;

  onSettingsChanged(s);
  }

