#include "graphicssettingswidget.h"

#include "listbox.h"
#include "lineedit.h"
#include <Tempest/Layout>

#ifdef __ANDROID__
GraphicsSettingsWidget::Settings::API GraphicsSettingsWidget::Settings::api
  = GraphicsSettingsWidget::Settings::openGL;
#else
GraphicsSettingsWidget::Settings::API GraphicsSettingsWidget::Settings::api
  = GraphicsSettingsWidget::Settings::openGL;
#endif

GraphicsSettingsWidget::Settings::Settings() {
  shadowMapRes = 1024;
  bloom        = Hight;
  glow         = true;
  normalMap    = true;

  shadowFilterQ = 2;
  oclusion      = true;
#ifdef __ANDROID__
  shadowMapRes = 512;
  bloom        = Hight;
  glow         = false;
  normalMap    = true;

  shadowFilterQ = 0;
  oclusion      = false;
#endif
  }

GraphicsSettingsWidget::GraphicsSettingsWidget(Resource &res) : Panel(res){
  layout().setMargin(8);

  resize( 200, 200 );
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
  b->setCurrentItem(2);
  if( s.shadowMapRes==512 )
    b->setCurrentItem(1);
  if( s.shadowMapRes==1024 )
    b->setCurrentItem(2);
  if( s.shadowMapRes==2048 )
    b->setCurrentItem(3);

  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::shadowMapRes);
  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"no sm filter");
  l.push_back(L"4x sm filter");
  l.push_back(L"8x sm filter");

  b->setItemList(l);
  b->setCurrentItem( s.shadowFilterQ );
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::smFilterQ );
  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"bloom off");
  l.push_back(L"bloom low");
  l.push_back(L"bloom hight");

  b->setItemList(l);
  b->setCurrentItem( s.bloom );
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::bloomQ );

  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"glow off");
  l.push_back(L"glow on");

  b->setItemList(l);
  b->setCurrentItem( s.glow?1:0 );
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::glow );

  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"normals off");
  l.push_back(L"normals on");

  b->setItemList(l);
  b->setCurrentItem( s.normalMap?1:0);
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::normalMap );
  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"oclusion off");
  l.push_back(L"oclusion on");

  b->setItemList(l);
  b->setCurrentItem( s.oclusion? 1:0 );
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::oclusion );
  layout().add(b);
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

void GraphicsSettingsWidget::glow(int v) {
  s.glow = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::normalMap(int v) {
  s.normalMap = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::oclusion(int v) {
  s.oclusion = v;

  onSettingsChanged(s);
  }

