#include "graphicssettingswidget.h"

#include "listbox.h"
#include "lineedit.h"
#include <Tempest/Layout>

GraphicsSettingsWidget::Settings::API GraphicsSettingsWidget::Settings::api
  = GraphicsSettingsWidget::Settings::directX;

GraphicsSettingsWidget::Settings::Settings() {
  shadowMapRes = 1024;
  bloom        = Hight;
  glow         = true;

  shadowFilterQ = 2;
  }

GraphicsSettingsWidget::GraphicsSettingsWidget(Resource &res) : Panel(res){
  layout().setMargin(8);

  resize( 200, 200 );
  setLayout( Tempest::Vertical );
  setDragable(1);

  ListBox *b = new ListBox(res);
  std::vector< std::wstring > l;
  l.push_back(L"sm 512x512");
  l.push_back(L"sm 1024x1024");
  l.push_back(L"sm 2048x2048");
  //l.push_back(L"sm 4096x4096");
  b->setItemList(l);
  b->setCurrentItem(1);
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::shadowMapRes);
  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"no sm filter");
  l.push_back(L"4x sm filter");
  l.push_back(L"8x sm filter");

  b->setItemList(l);
  b->setCurrentItem(2);
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::smFilterQ );
  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"bloom off");
  l.push_back(L"bloom low");
  l.push_back(L"bloom hight");

  b->setItemList(l);
  b->setCurrentItem(2);
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::bloomQ );

  layout().add(b);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"glow off");
  l.push_back(L"glow on");

  b->setItemList(l);
  b->setCurrentItem(1);
  b->onItemSelected.bind(*this, &GraphicsSettingsWidget::glow );

  layout().add(b);
  }

void GraphicsSettingsWidget::shadowMapRes(int v) {
  int r[] = {512, 1024, 2048, 4096};
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

