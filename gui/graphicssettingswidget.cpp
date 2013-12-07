#include "graphicssettingswidget.h"

#include "listbox.h"
#include "lineedit.h"
#include "checkbox.h"

#include <Tempest/Layout>
#include <Tempest/Android>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <iostream>

#include <tuple>

#ifdef __ANDROID__
GraphicsSettingsWidget::Settings::API GraphicsSettingsWidget::Settings::api
  = GraphicsSettingsWidget::Settings::openGL;
#else
GraphicsSettingsWidget::Settings::API GraphicsSettingsWidget::Settings::api
  = GraphicsSettingsWidget::Settings::openGL;
#endif

GraphicsSettingsWidget::Settings GraphicsSettingsWidget::Settings::st;

Tempest::signal<const GraphicsSettingsWidget::Settings&>
  GraphicsSettingsWidget::onSettingsChanged;

GraphicsSettingsWidget::Settings::Settings() {
  oreentation        = 0;
  physics            = 4;
  shadowMapRes       = 1024;
  bloom              = Hight;
  glow               = true;
  normalMap          = true;

  shadowFilterQ      = 2;
  oclusion           = true;
  shadowTextures     = true;
  transcurentShadows = true;
  atest              = true;
#ifdef __ANDROID__
  physics            = 1;
  shadowMapRes       = 512;
  bloom              = Off;
  glow               = false;
  normalMap          = true;

  shadowFilterQ      = 0;
  oclusion           = false;
  shadowTextures     = false;
  transcurentShadows = false;
  atest              = false;
#endif
  }

bool GraphicsSettingsWidget::
  Settings::operator ==(const GraphicsSettingsWidget::Settings &s) const {
  return std::tie(bloom,
                  shadowMapRes,
                  shadowFilterQ,
                  glow,
                  normalMap,
                  oclusion,
                  shadowTextures,
                  transcurentShadows,
                  physics,
                  oreentation,
                  atest ) ==
         std::tie(s.bloom,
                  s.shadowMapRes,
                  s.shadowFilterQ,
                  s.glow,
                  s.normalMap,
                  s.oclusion,
                  s.shadowTextures,
                  s.transcurentShadows,
                  s.physics,
                  s.oreentation,
                  s.atest );
  }

void GraphicsSettingsWidget::Settings::setSettings( const Settings &s) {
  if( s==st )
    return;

  st = s;
  onSettingsChanged(st);
  }

const GraphicsSettingsWidget::Settings &GraphicsSettingsWidget::Settings::settings(){
  return st;
  }

const std::string& GraphicsSettingsWidget::Settings::settingsFileName() {
#ifdef __ANDROID__
  static std::string s = std::string(Tempest::AndroidAPI::internalStorage()) +
                         "settings.js";
#else
  static std::string s = "./settings.js";
#endif
  return s;
  }

void GraphicsSettingsWidget::Settings::save() {
  using namespace rapidjson;

  GenericStringBuffer<UTF8<>> buffer;
  PrettyWriter<GenericStringBuffer<UTF8<>>> writer(buffer);

  Document doc;
  doc.SetObject();

  doc.AddMember("shadowMapQ",    st.shadowMapRes,  doc.GetAllocator() );
  doc.AddMember("shadowFilterQ", st.shadowFilterQ, doc.GetAllocator() );
  doc.AddMember("glow",          st.glow,          doc.GetAllocator() );
  doc.AddMember("normalMap",     st.normalMap,     doc.GetAllocator() );
  doc.AddMember("atest",         st.atest,         doc.GetAllocator() );

  doc.AddMember("physics",       st.physics,     doc.GetAllocator() );
  doc.AddMember("oreentation",   st.oreentation, doc.GetAllocator() );

  doc.Accept( writer );

  std::ofstream fout( settingsFileName().c_str(), std::ofstream::binary );
  fout.write( buffer.GetString(), buffer.Size() );
  }

bool GraphicsSettingsWidget::Settings::load() {
  using namespace rapidjson;

  std::ifstream is( settingsFileName().c_str(), std::ifstream::binary );
  if( !is )
    return 0;

  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  std::string str;
  str.resize( length );
  is.read ( &str[0], length );

  if( !is )
    return 0;
  is.close();

  Document d;
  d.Parse<0>( str.data());

  if( d.HasParseError() ){
    size_t pe = d.GetErrorOffset(), pl = 1, pch = 0;
    for( size_t i=0; i<pe && i<str.size(); ++i ){
      pch++;
      if( str[i]=='\n' ){
        pch = 0;
        ++pl;
        }
      }

    std::cout << "[settings, " << pl <<":" <<pch <<"]: ";
    std::cout << d.GetParseError() << std::endl;
    return 0;
    }

  if( !d.IsObject() )
    return 0;

  if( d["shadowMapQ"].IsInt() )
    st.shadowMapRes = d["shadowMapQ"].GetInt();
  if( d["shadowFilterQ"].IsInt() )
    st.shadowFilterQ = d["shadowFilterQ"].GetInt();

  if( d["glow"].IsBool() )
    st.glow = d["glow"].GetBool();
  if( d["normalMap"].IsBool() )
    st.normalMap = d["normalMap"].GetBool();
  if( d["atest"].IsBool() )
    st.atest = d["atest"].GetBool();

  if( d["physics"].IsInt() )
    st.physics = d["physics"].GetInt();

  if( d["oreentation"].IsInt() )
    st.oreentation = d["oreentation"].GetInt();

  onSettingsChanged(st);
  return 1;
  }

GraphicsSettingsWidget::GraphicsSettingsWidget(Resource &res) : Panel(res){
  Settings s = Settings::st;

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

  CheckBox *ck = new CheckBox(res);
  ck->setText("colored shadows");
  ck->setChecked( s.transcurentShadows );
  ck->checked.bind( this, &GraphicsSettingsWidget::colorSh );
  layout().add(ck);

  b = new ListBox(res);
  l.clear();
  l.push_back(L"bloom off");
  l.push_back(L"bloom low");
  l.push_back(L"bloom hight");

  b->setItemList(l);
  b->setCurrentItem( s.bloom );
  b->onItemSelected.bind( this, &GraphicsSettingsWidget::bloomQ );

  layout().add(b);

  ck = new CheckBox(res);
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
  Settings s = Settings::st;
  int r[] = {0, 512, 1024, 2048, 4096};
  s.shadowMapRes = r[v];

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::smFilterQ(int v) {
  Settings s = Settings::st;
  s.shadowFilterQ = v;
  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::colorSh(bool v) {
  Settings s = Settings::st;
  s.transcurentShadows = v;
  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::bloomQ(int v) {
  Settings s = Settings::st;
  s.bloom = Settings::Bloom(v);

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::glow(bool v) {
  Settings s = Settings::st;
  s.glow = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::normalMap(bool v) {
  Settings s = Settings::st;
  s.normalMap = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::oclusion(bool v) {
  Settings s = Settings::st;
  s.oclusion = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::shTexture(bool v) {
  Settings s = Settings::st;
  s.shadowTextures = v;

  onSettingsChanged(s);
  }

void GraphicsSettingsWidget::update() {
  layout().removeAll();

  }

