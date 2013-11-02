#include "gamesettings.h"

#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <Tempest/Android>

int GameSettings::difficulty = 1;
Tempest::Color GameSettings::color = Tempest::Color(1,1,0,1);
bool GameSettings::smallMenu = false;

GameSettings::GameSettings() {

  }

void GameSettings::save(const char *file) {
  using namespace rapidjson;

  GenericStringBuffer<UTF8<>> buffer;
  PrettyWriter<GenericStringBuffer<UTF8<>>> writer(buffer);

  Document doc;
  doc.SetObject();

  doc.AddMember("difficulty",    difficulty,  doc.GetAllocator() );

  Value cl;
  cl.SetObject();
  cl.AddMember("r", color.r(), doc.GetAllocator() );
  cl.AddMember("g", color.g(), doc.GetAllocator() );
  cl.AddMember("b", color.b(), doc.GetAllocator() );
  cl.AddMember("a", color.a(), doc.GetAllocator() );

  doc.AddMember("color",         cl,         doc.GetAllocator() );
  doc.AddMember("smallMenu",     smallMenu,  doc.GetAllocator() );

  doc.Accept( writer );

  std::ofstream fout( file, std::ofstream::binary );
  fout.write( buffer.GetString(), buffer.Size() );
  }

void GameSettings::load(const char *file) {
  using namespace rapidjson;

  std::ifstream is( file, std::ifstream::binary );
  if( !is )
    return;

  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  std::string str;
  str.resize( length );
  is.read ( &str[0], length );

  if( !is )
    return;
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
    return;
    }

  if( !d.IsObject() )
    return;

  if( d["difficulty"].IsInt() )
    difficulty = d["difficulty"].GetInt();

  if( d["smallMenu"].IsInt() )
    smallMenu = d["smallMenu"].GetInt();

  if( d["color"].IsObject() ){
    const Value& cl = d["color"];

    const char*  lt[] = {"r", "g", "b", "a"};
    for( int i=0; i<4; ++i )
      if( cl[ lt[i] ].IsNumber() )
        color[i] = cl[ lt[i] ].GetDouble();
    }
  }

void GameSettings::save() {
  save( fileName().c_str() );
  }

void GameSettings::load() {
  load( fileName().c_str() );
  }

const std::string &GameSettings::fileName() {
#ifdef __ANDROID__
  static std::string s = std::string(Tempest::AndroidAPI::internalStorage()) +
                         "gameSettings.js";
#else
  static std::string s = "./gameSettings.js";
#endif
  return s;
  }
