#include "prototypesloader.h"

#include <Tempest/Shortcut>
#include "util/lexicalcast.h"

#include <iostream>
#include "algo/algo.h"

#include <rapidjson/document.h>

#include <Tempest/SystemAPI>
#include <Tempest/Assert>
#include <cmath>

PrototypesLoader::PrototypesLoader() {
  defs.reserve(4);
  }

const ProtoObject &PrototypesLoader::get(const std::string &obj) const {
  auto i = defs.back().data.find(obj);

  if( i!=defs.back().data.end() ){
    return *i->second.get();
    } else {
    T_ASSERT(0);
    }

  return *defs.back().data.begin()->second.get();
  }

std::vector< PrototypesLoader::PProtoObject>
    PrototypesLoader::allClasses() const {
  std::vector< PrototypesLoader::PProtoObject > obj;

  for( auto i = defs.back().data.begin(); i!=defs.back().data.end(); ++i )
    obj.push_back( i->second );

  std::sort( obj.begin(), obj.end(), cmp );

  return obj;
  }

const Spell &PrototypesLoader::spell(const std::string &obj) const {
  auto i = defs.back().dataSpells.find(obj);

  if( i!=defs.back().dataSpells.end() ){
    return *i->second.get();
    } else {
    T_ASSERT(0);
    }

  return *defs.back().dataSpells.begin()->second.get();
  }

const Upgrade &PrototypesLoader::upgrade(const std::string &obj) const {
  auto i = defs.back().dataUpgrades.find(obj);

  if( i!=defs.back().dataUpgrades.end() ){
    return *i->second.get();
    } else {
    T_ASSERT(0);
    }

  return *defs.back().dataUpgrades.begin()->second.get();
  }

const ParticleSystemDeclaration &PrototypesLoader::particle(const std::string &obj) const {
  auto i = defs.back().dataParticles.find(obj);

  if( i!=defs.back().dataParticles.end() ){
    return *i->second.get();
    } else {
    T_ASSERT(0);
    }

  return *defs.back().dataParticles.begin()->second.get();
  }

void PrototypesLoader::unload() {
  defs.pop_back();
  }

const std::string &PrototypesLoader::material(int i) {
  return defs.back().materials[i];
  }

size_t PrototypesLoader::material(const char *c) const {
  size_t id = find( defs.back().materials, c )-defs.back().materials.begin();

  if( id==defs.back().materials.size() )
    defs.back().materials.push_back(c);

  return id;
  }

size_t PrototypesLoader::atackId( const std::string &str ) {
  size_t ret = 0;
  auto i = std::find( defs.back().atackTypes.begin(),
                      defs.back().atackTypes.end(), str );
  if( i!=defs.back().atackTypes.end() ){
    ret  = i-defs.back().atackTypes.begin();
    } else {
    ret = defs.back().atackTypes.size();
    defs.back().atackTypes.push_back(str);
    }

  return ret;
  }

void PrototypesLoader::readCommandsPage( ProtoObject &obj,
                                         const rapidjson::Value &v ) {
  using namespace rapidjson;

  int p = 0;

  if( v["num"].IsInt() ){
    p = v["num"].GetInt();

    if( p>=0 && size_t(p)>= obj.commands.pages.size() )
      obj.commands.pages.resize(p+1);
    }

  if( v["button"].IsArray() ){
    const Value& b = v["button"];
    for( size_t i=0; i<b.Size(); ++i ){
      ProtoObject::CmdButton btn;
      readButton( btn, b[i] );
      obj.commands.pages[p].btn.push_back(btn);
      }
    }

  if( v["button"].IsObject() ){
    ProtoObject::CmdButton btn;
    readButton( btn, v["button"] );
    obj.commands.pages[p].btn.push_back(btn);
    }
  }

void PrototypesLoader::readButton( ProtoObject::CmdButton &b,
                                   const rapidjson::Value &v ) {
  using namespace rapidjson;

  b.action = ProtoObject::CmdButton::NoAction;
  readIf( v["icon"],  b.icon  );

  if( v["action"].IsString() ){
    std::string str = v["action"].GetString();
    if( str=="buy" )
      b.action = ProtoObject::CmdButton::Buy; else
    if( str=="build" )
      b.action = ProtoObject::CmdButton::Build; else
    if( str=="page" )
      b.action = ProtoObject::CmdButton::Page; else
    if( str=="upgrade" )
      b.action = ProtoObject::CmdButton::Upgrade; else
    if( str=="castToGround" )
      b.action = ProtoObject::CmdButton::CastToGround;
    }

  readIf( v["taget"], b.taget );

  readIf( v["x"], b.x, 0 );
  readIf( v["y"], b.y, 0 );

  if( v["hotkey"].IsString() &&
      v["hotkey"].GetStringLength() ){
    char ch = v["hotkey"].GetString()[0];
    b.hotkey = Tempest::KeyEvent::KeyType( Tempest::KeyEvent::K_A + ch - 'a' );
    }

  if( v["hint"].IsString() ){
    std::string str = v["hint"].GetString();
    b.hint.assign( str.begin(), str.end() );
    }
  }

bool PrototypesLoader::cmp( const std::shared_ptr<ProtoObject> &a,
                            const std::shared_ptr<ProtoObject> &b ) {
  return (a->name < b->name );
  }

void PrototypesLoader::loadClass(const rapidjson::Value &v) {
  using namespace rapidjson;

  if( !v["name"].IsString() )
    return;
  std::string name = v["name"].GetString();

  PProtoObject p = PProtoObject( new ProtoObject() );
  if( v["extends"].IsString() ){
    *p = get( v["extends"].GetString() );
    }

  p->name = name;
  defs.back().data[ name ] = p;

  const Value& speed = v["speed"];
  if( speed.IsObject() ){
    if( speed["move"].IsInt() )
      p->data.speed = speed["move"].GetInt();
    if( speed["rotate"].IsInt() )
      p->rotateSpeed = speed["rotate"].GetInt();

    if( speed["acseleration"].IsInt() ){
      p->data.acseleration = speed["acseleration"].GetInt();
      }
    }

  const Value& view = v["view"];
  if( view.IsObject() ){
    p->view.push_back( ProtoObject::View() );
    readView( p->view.back(), view );
    }

  if( view.IsArray() ){
    for( size_t i=0; i<view.Size(); ++i )
      if( view[i].IsObject() ){
        p->view.push_back( ProtoObject::View() );
        readView( p->view.back(), view[i] );
        }
    }

  const Value& behavior = v["behavior"];
  if( behavior.IsObject() ){
    p->behaviors.clear();
    readBehavior( *p, behavior );
    }

  if( behavior.IsArray() ){
    p->behaviors.clear();
    for( size_t i=0; i<behavior.Size(); ++i ){
      readBehavior( *p, behavior[i] );
      }
    }

  const Value& ability = v["ability"];
  if( ability.IsArray() ){
    for( size_t i=0; i<ability.Size(); ++i ){
      const Value& v = ability[i];
      if( v.IsString() ){
        p->ability.push_back( v.GetString() );
        }
      }
    }

  const Value& e = v["property"];
  if( e.IsObject() )
    readProperty( *p, e );

  if( e.IsArray() ){
    for( size_t i=0; i<e.Size(); ++i )
      if( e[i].IsObject() )
        readProperty( *p, e[i] );
    }

  const Value& s = v["view_size"];
  if( s.IsObject() ){
    for( int i=0; i<3; ++i ){
      readIf( s["min"],      p->sizeBounds.min[i] );
      readIf( s["max"],      p->sizeBounds.max[i] );
      }

    readIf( s["minX"],      p->sizeBounds.min[0] );
    readIf( s["minY"],      p->sizeBounds.min[1] );
    readIf( s["minZ"],      p->sizeBounds.min[2] );

    readIf( s["maxX"],      p->sizeBounds.max[0] );
    readIf( s["maxY"],      p->sizeBounds.max[1] );
    readIf( s["maxZ"],      p->sizeBounds.max[2] );
    }

  if( v["atack"].IsObject() ){
    p->data.atk.clear();
    ProtoObject::GameSpecific::Atack a;
    readAtack(a, v["atack"]);
    p->data.atk.push_back( a );
    } else
  if( v["atack"].IsArray() ){
    p->data.atk.clear();
    const Value& atk = v["atack"];
    for( size_t i=0; i<atk.Size(); ++i )
      if( atk[i].IsObject() ){
        ProtoObject::GameSpecific::Atack a;
        readAtack(a, atk[i] );
        p->data.atk.push_back( a );
        }
    }

  if( v["commands"].IsObject() )
    readCommands( *p, v["commands"] );
  }

void PrototypesLoader::readCommands( ProtoObject &obj,
                                     const rapidjson::Value &v ){
  using namespace rapidjson;
  const Value &p = v["page"];

  if( p.IsArray() ){
    for( size_t i=0; i<p.Size(); ++i ){
      readCommandsPage(obj, p[i]);
      }
    }

  if( p.IsObject() )
    readCommandsPage(obj, p );
  }

void PrototypesLoader::readBehavior( ProtoObject &p,
                                     const rapidjson::Value &v ) {
  if( v.IsObject() ){
    static const std::string add = "add";
    if( v.MemberBegin()->name.GetString()==add ){
      p.behaviors.push_back( v.MemberBegin()->value.GetString() );
      } else {
      remove( p.behaviors, v.MemberBegin()->value.GetString() );
      }
    }
  }

void PrototypesLoader::readView( ProtoObject::View &v,
                                 const rapidjson::Value& e ) {
  using namespace rapidjson;

  std::fill(v.align, v.align+3, 0);

  if( e["src"].IsString() )
    v.name = e["src"].GetString();

  v.materials.push_back( "shadow_cast" );
  v.materials.push_back( "phong" );

  if( e["modelAlign"].IsString() ){
    const char* tmp = e["modelAlign"].GetString();
    if( e["modelAlign"].GetStringLength()==3 ){
      for( int i=0; i<3; ++i ){
        if( tmp[i]=='i' )
          v.align[i] =  1; else
        if( tmp[i]=='d' )
          v.align[i] = -1; else
        if( tmp[i]=='I' )
          v.align[i] =  2; else
        if( tmp[i]=='D' )
          v.align[i] = -2;
        }
      }
    }

  readIf( e["alignSize"], v.alignSize, 0.5 );
  readIf( e["isParticle"], v.isParticle );

  readIf( e["isLandDecal"],  v.isLandDecal  );
  readIf( e["hasOverDecal"], v.hasOverDecal );

  readIf( e["specular"], v.specularFactor, 0.5 );
  readIf( e["randRotate"], v.randRotate, false );

  std::string shadowT = "basic";
  readIf( e["shadowType"], shadowT );

  if( shadowT=="basic" )
    v.shadowType = ProtoObject::View::BasicShadow;

  if( shadowT=="noShadow" )
    v.shadowType = ProtoObject::View::NoShadow;

  if( shadowT=="bias" )
    v.shadowType = ProtoObject::View::BiasShadow;

  if( e["size"].IsNumber() ){
    double tmp = e["size"].GetDouble();
    std::fill( v.size, v.size+3, tmp );
    } else {
    readIf( e["sizeX"], v.size[0], 1 );
    readIf( e["sizeY"], v.size[1], 1 );
    readIf( e["sizeZ"], v.size[2], 1 );
    }

  if( e["physic"].IsString() ){
    static const std::string box    = "box";
    static const std::string sphere = "sphere";

    v.physModel = ProtoObject::View::NoModel;

    if( box==e["physic"].GetString() ){
      v.physModel = ProtoObject::View::Box;
      if( e["boxSize"].IsNumber() ){
        double s = e["boxSize"].GetDouble();
        std::fill( v.boxSize, v.boxSize+3, s );
        } else {
        readIf( e["boxSizeX"], v.boxSize[0] );
        readIf( e["boxSizeY"], v.boxSize[1] );
        readIf( e["boxSizeZ"], v.boxSize[2] );
        }
      } else
    if( sphere==e["physic"].GetString() ){
      v.physModel = ProtoObject::View::Sphere;
      readIf( e["diameter"], v.sphereDiameter, 1 );
      }
    }

  if( e["material"].IsObject() ){
    readMaterial( v, e["material"] );
    }

  if( e["material"].IsArray() ){
    const Value& m = e["material"];
    for( size_t i=0; i<m.Size(); ++i )
      readMaterial( v, m[i] );
    }

  if( e["shadedMaterial"].IsString() ){
    const char* c    = e["shadedMaterial"].GetString();
    v.shadedMaterial = material(c);
    }
  }

void PrototypesLoader::readProperty( ProtoObject &p,
                                     const rapidjson::Value &e){
  using namespace rapidjson;

  readIf( e["size"], p.data.size );
  p.data.size = std::max(1, p.data.size);

  readIf( e["visionRange"], p.data.visionRange );
  p.data.visionRange = std::max(1, p.data.visionRange);

  readIf( e["maxHp"], p.data.maxHp );
  p.data.maxHp = std::max(1, p.data.maxHp);

  readIf( e["gold"],      p.data.gold      );
  readIf( e["lim"],       p.data.lim       );
  readIf( e["limInc"],    p.data.limInc    );
  readIf( e["buildTime"], p.data.buildTime );
  readIf( e["armor"],     p.data.armor     );

  readIf( e["isBackground"], p.data.isBackground );
  readIf( e["isDynamic"],    p.data.isDynamic );
  readIf( e["invincible"],   p.data.invincible   );

  if( e["mouseHint"].IsString() ){
    std::string s = e["mouseHint"].GetString();
    p.mouseHint.assign( s.begin(), s.end() );
    }

  if( e["type"].IsArray() ){
    const Value& t = e["type"];
    for( size_t i=0; i<t.Size(); ++i )
      if( t[i].IsString() ){
        const char* str = t[i].GetString();
        auto i = std::find( defs.back().unitTypes.begin(),
                            defs.back().unitTypes.end(), str );

        size_t id = defs.back().unitTypes.size();
        if( i!=defs.back().unitTypes.end() ){
          id = i-defs.back().unitTypes.begin();
          } else {
          defs.back().unitTypes.push_back(str);
          }

        p.data.utype.push_back(id);
        }
    }

  if( e["user"].IsObject() ){
    const Value& u = e["user"];
    for( Value::ConstMemberIterator m = u.MemberBegin(); m!=u.MemberEnd(); ++m ){
      if( m->value.IsString() ){
        p.data.propertyStr[ m->name.GetString() ] = m->value.GetString();
        }
      }
    }

  if( e["deathAnim"].IsString() ){
    p.deathAnim = ProtoObject::NoAnim;

    static const std::string ph = "physic";
    if( e["deathAnim"].GetString()==ph )
      p.deathAnim = ProtoObject::Physic;
    }

  if( e["deathExplosion"].IsArray() ){
    const Value& u = e["deathExplosion"];

    for( size_t i=0; i<u.Size(); ++i )
      if( u[i].IsString() )
        p.deathExplosion.push_back( u[i].GetString() );
    }
  }

void PrototypesLoader::readMaterial( ProtoObject::View &v,
                                     const rapidjson::Value& e ) {
  using namespace rapidjson;
  static const std::string add = "add";

  if( e.MemberBegin()+1==e.MemberEnd() &&
      e.MemberBegin()->value.IsString() ){
    std::string tmp = e.MemberBegin()->value.GetString();

    if( e.MemberBegin()->name.GetString()==add ){
      v.materials.push_back( tmp );
      } else {
      remove( v.materials, tmp );
      }
    }
  }

void PrototypesLoader::readAtack( ProtoObject::GameSpecific::Atack &a,
                                  const rapidjson::Value& e ) {
  using namespace rapidjson;

  a.damage = 0;
  a.delay  = 0;
  a.range  = 0;
  a.splashSize   = 0;
  a.splashDamage = 0;

  if( e["damage"].IsInt() ){
    a.damage       = e["damage"].GetInt();
    a.splashDamage = a.damage;
    }
  readIf( e["range"],  a.range      );
  readIf( e["delay"],  a.delay      );
  readIf( e["splash"], a.splashSize );

  readIf( e["slpashDamage"], a.splashDamage );

  if( e["taget"].IsString() ){
    std::string str = e["taget"].GetString();
    auto i = std::find( defs.back().unitTypes.begin(),
                        defs.back().unitTypes.end(), str );
    if( i!=defs.back().unitTypes.end() )
      a.uDestType  = i-defs.back().unitTypes.begin();
    }

  if( e["type"].IsString() ){
    std::string str = e["type"].GetString();
    a.type = atackId(str);
    }

  if( e["bullet"].IsString() ){
    a.bullet = e["bullet"].GetString();
    }
  if( e["explosion"].IsString() ){
    a.explosion = e["explosion"].GetString();
    }
  }

void PrototypesLoader::readIf( const rapidjson::Value &v,
                               std::string &val,
                               const std::string& def) {
  if( v.IsString() )
    val = v.GetString(); else
    val = def;
  }

void PrototypesLoader::readIf(const rapidjson::Value &v, int &val, int def) {
  if( v.IsInt() )
    val = v.GetInt(); else
    val = def;
  }

void PrototypesLoader::readIf( const rapidjson::Value &v,
                               double &val, double def) {
  if( v.IsNumber() )
    val = v.GetDouble(); else
    val = def;
  }

void PrototypesLoader::readIf( const rapidjson::Value &v,
                               float &val, float def) {
  if( v.IsNumber() )
    val = v.GetDouble(); else
    val = def;
  }

void PrototypesLoader::readIf(const rapidjson::Value &v, bool &val, bool def) {
  if( v.IsBool() )
    val = v.GetBool(); else
  if( v.IsInt() )
    val = v.GetInt(); else
    val = def;
  }

void PrototypesLoader::readSpell( Spell &v,
                                  const rapidjson::Value& e ){
  using namespace rapidjson;

  const Value& prop = e["property"];
  if( prop.IsObject() ){
    readIf( prop["coolDown"],    v.coolDown    );
    readIf( prop["manaCost"],    v.manaCost    );
    readIf( prop["bulletSpeed"], v.bulletSpeed );
    readIf( prop["autoCast"],    v.autoCast    );
    }

  const Value& info = e["info"];
  if( info.IsObject() ){
    std::string str;
    readIf( info["type"], str );
    if( str=="castToUnit" )
      v.mode = Spell::CastToUnit;
    if( str=="castToCoord" )
      v.mode = Spell::CastToCoord;
    }
  }

void PrototypesLoader::readGrade( Upgrade &v,
                                  const rapidjson::Value& e ){
  using namespace rapidjson;

  const Value& p = e["property"];
  if( p.IsObject() ){
    readIf( p["gold"],      v.data[0].gold );
    readIf( p["buildTime"], v.data[0].buildTime );
    readIf( p["lvCount"],   v.lvCount );
    }

  if( e["atk_target"].IsString() ){
    v.id = atackId( e["atk_target"].GetString() );
    }
  }

void PrototypesLoader::readParticle( ParticleSystemDeclaration &obj,
                                     const rapidjson::Value& e  ) {
  if( e["init"].IsObject() ){
    if( e["init"]["min"].IsObject() )
      readParticle( obj.initMin, e["init"]["min"] );

    if( e["init"]["max"].IsObject() )
      readParticle( obj.initMax, e["init"]["max"] );
    }

  if( e["velocity"].IsObject() ){
    if( e["velocity"]["min"].IsObject() )
      readParticle( obj.dmin, e["velocity"]["min"] );

    if( e["velocity"]["max"].IsObject() )
      readParticle( obj.dmax, e["velocity"]["max"] );
    }

  if( e["acceleration"].IsObject() ){
    obj.hasDD = true;

    if( e["acceleration"]["min"].IsObject() )
      readParticle( obj.ddmin, e["acceleration"]["min"] );

    if( e["acceleration"]["max"].IsObject() )
      readParticle( obj.ddmax, e["acceleration"]["max"] );
    }

  if( e["density"].IsInt() )
    obj.density = e["density"].GetInt();

  if( e["emit"].IsObject() ){

    }

  if( e["options"].IsObject() ){
    if( e["options"]["orign"].IsString() ){
      const char* orign = e["options"]["orign"].GetString();
      static const std::string
          xy     = "xy",
          screen = "screen",
          planar = "planar";

      if( orign==xy )
        obj.orign = ParticleSystemDeclaration::XYOrign;
      if( orign==screen )
        obj.orign = ParticleSystemDeclaration::Screen;
      if( orign==planar )
        obj.orign = ParticleSystemDeclaration::Planar;
      }
    }
  }

void PrototypesLoader::readParticle( ParticleSystemDeclaration::D &obj,
                                     const rapidjson::Value& e ) {
  readIf( e["x"], obj.x );
  readIf( e["y"], obj.y );
  readIf( e["z"], obj.z );

  readIf( e["size"],  obj.size );
  readIf( e["angle"], obj.angle );
  obj.angle = obj.angle*M_PI/180.0;

  readIf( e["r"], obj.r );
  readIf( e["g"], obj.g );
  readIf( e["b"], obj.b );
  readIf( e["a"], obj.a );
  }

void PrototypesLoader::load(const std::string &s) {
  if( defs.size() ){
    defs.push_back( defs.back() );
    } else {
    defs.push_back( Defs() );
    defs.back().atackTypes.push_back("");
    }

  loadImpl(s);
  }

void PrototypesLoader::loadImpl(const std::string &s) {
  using namespace rapidjson;
  std::string jsonstr = Tempest::SystemAPI::loadText(s.data());

  Document d;
  d.Parse<0>(jsonstr.data());

  if( d.HasParseError() ){
    size_t pe = d.GetErrorOffset(), pl = 1, pch = 0;
    for( size_t i=0; i<pe && i<jsonstr.size(); ++i ){
      pch++;
      if( jsonstr[i]=='\n' ){
        pch = 0;
        ++pl;
        }
      }

    std::cout << "[" << s <<", " << pl <<":" <<pch <<"]: ";
    std::cout << d.GetParseError() << std::endl;
    return;
    }

  const Value& m = d["module"];
  if( m.IsArray() ){
    for( size_t i=0; i<m.Size(); ++i )
      if( m[i].IsObject() &&
          m[i]["file"].IsString() ){
        loadImpl( m[i]["file"].GetString() );
        }
    }

  const Value& cls = d["class"];
  if( cls.IsArray() ){
    for( size_t i=0; i<cls.Size(); ++i ){
      if( cls[i].IsObject() )
        loadClass( cls[i] );
      }
    }

  const Value& pp = d["particle"];
  if( pp.IsArray() ){
    for( size_t i=0; i<pp.Size(); ++i )
      if( pp[i].IsObject() &&
          pp[i]["name"].IsString() ){
        PParticle p = PParticle( new ParticleSystemDeclaration() );
        //p->name = name;
        defs.back().dataParticles[ pp[i]["name"].GetString() ] = p;
        readParticle( *p, pp[i] );
        }
    }

  const Value& sp = d["spell"];
  if( sp.IsArray() ){
    for( size_t i=0; i<sp.Size(); ++i )
      if( sp[i].IsObject() &&
          sp[i]["name"].IsString() ){
        PSpell p = PSpell( new Spell() );
        p->name = sp[i]["name"].GetString();
        defs.back().dataSpells[ p->name ] = p;
        p->id = defs.back().spells.size();
        defs.back().spells.push_back( p );
        readSpell( *p, sp[i] );
        }
    }

  const Value& up = d["upgrade"];
  if( up.IsArray() ){
    for( size_t i=0; i<up.Size(); ++i )
      if( up[i].IsObject() &&
          up[i]["name"].IsString() ){
        PUpgrade p = PUpgrade( new Upgrade() );
        p->name = up[i]["name"].GetString();
        defs.back().dataUpgrades[ p->name ] = p;
        p->id = defs.back().grades.size();
        defs.back().grades.push_back( p );

        readGrade( *p.get(), up[i] );
        }
    }
  }
