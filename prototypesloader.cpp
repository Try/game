#include "prototypesloader.h"

#include <MyWidget/Shortcut>
#include <tinyxml.h>
#include "util/lexicalcast.h"

#include <iostream>
#include "algo/algo.h"

PrototypesLoader::PrototypesLoader() {
  }

const ProtoObject &PrototypesLoader::get(const std::string &obj) const {
  auto i = data.find(obj);

  if( i!=data.end() ){
    return *i->second.get();
    } else {
    assert(0);
    }
  }

std::vector< PrototypesLoader::PProtoObject>
    PrototypesLoader::allClasses() const {
  std::vector< PrototypesLoader::PProtoObject > obj;

  for( auto i = data.begin(); i!=data.end(); ++i )
    obj.push_back( i->second );

  std::sort( obj.begin(), obj.end(), cmp );

  return obj;
  }

const Spell &PrototypesLoader::spell(const std::string &obj) const {
  auto i = dataSpells.find(obj);

  if( i!=dataSpells.end() ){
    return *i->second.get();
    } else {
    assert(0);
    }
  }

void PrototypesLoader::readElement(TiXmlNode *node) {
  if ( !node )
    return;

  if ( node->Type()==TiXmlNode::TINYXML_ELEMENT ) {
    std::string type = node->Value();

    if( type=="module"){
      std::string file;

      if( find(node->ToElement(), "file", file) ){
        load(file);
        } else {
        error("file not specified");
        }

      if( node->FirstChild()!=0 )
        error("invalid moule declaration");
      } else

    if( type=="class"){
      std::string name;

      if( find(node->ToElement(), "name", name) ){
        PProtoObject p = PProtoObject( new ProtoObject() );
        p->name = name;
        data[ name ] = p;

        for ( TiXmlNode* n = node->FirstChild(); n != 0; n = n->NextSibling() ){
          readClassMember( *p.get(), n );
          }

        } else {
        error("class name not specified");
        }
      } else

    if( type=="spell"){
      std::string name;

      if( find(node->ToElement(), "name", name) ){
        PSpell p = PSpell( new Spell() );
        p->name = name;
        dataSpells[ name ] = p;
        p->id = spells.size();
        spells.push_back( p );

        for ( TiXmlNode* n = node->FirstChild(); n != 0; n = n->NextSibling() ){
          readSpellMember( *p.get(), n );
          }

        } else {
        error("spell name not specified");
        }

      } else

    dumpAttribs(node->ToElement() );
    }
/*
  for ( TiXmlNode* n = node->FirstChild(); n != 0; n = n->NextSibling() ){
    readElement( n );
    } */
}

void PrototypesLoader::readClassMember( ProtoObject &obj, TiXmlNode *node) {
  if ( !node )
    return;

  TiXmlElement * e = node->ToElement();

  if ( node->Type()==TiXmlNode::TINYXML_ELEMENT ) {
    std::string type = node->Value();

    if( type=="speed" ){
      const std::string err1 = "object rotate speed not declarated, set to 5.0";
      const std::string err2 = "object move   speed not declarated, set to 15";

      obj.rotateSpeed = Lexical::cast<double>( findStr( e, "rotate", "5", err1 ) );
      obj.data.speed  = Lexical::cast<int>( findStr( e, "move", "15", err2 ) );
      }

    if( type=="view" ){
      ProtoObject::View v;
      std::fill(v.align, v.align+3, 0);

      v.name = findStr( e, "src", "invalid view name");
      v.materials.push_back( "shadow_cast" );
      v.materials.push_back( "phong" );

      std::string tmp;
      if( find( e, "modelAlign", tmp) ){
        if( tmp.size()==3 ){
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
          } else {
          error("invalid model align");
          }
        }

      if( find(e, "alignSize", tmp ) ){
        v.alignSize = Lexical::cast<double>(tmp);
        } else
        v.alignSize = 0.5;

      if( find( e, "isParticle", tmp) ){
        v.isParticle =  !(tmp=="false" || tmp=="0");
        } else {
        v.isParticle = false;
        }

      if( find( e, "specular", tmp) ){
        v.specularFactor = Lexical::cast<double>(tmp);
        } else {
        v.specularFactor = 0.5;
        }

      if( find( e, "randRotate", tmp) ){
        v.randRotate = !(tmp=="false" || tmp=="0");
        } else {
        v.randRotate = false;
        }

      if( find( e, "size", tmp) ){
        std::fill( v.size, v.size+3, Lexical::cast<double>( tmp ) );
        } else {
        v.size[0] = Lexical::cast<double>( findStr( e, "sizeX", "1", "" ) );
        v.size[1] = Lexical::cast<double>( findStr( e, "sizeY", "1", "" ) );
        v.size[2] = Lexical::cast<double>( findStr( e, "sizeZ", "1", "" ) );
        }

      if( find( e, "physic", tmp) ){
        if( tmp=="box" ){
          const std::string err =  "box size component not declarated, set to 1.0";
          v.physModel = ProtoObject::View::Box;

          std::string tmp;
          if( find( e, "boxSize", tmp) ){
            v.boxSize[0] = Lexical::cast<double>( tmp );
            v.boxSize[1] = v.boxSize[0];
            v.boxSize[2] = v.boxSize[0];
            } else {
            v.boxSize[0] = Lexical::cast<double>( findStr( e, "boxSizeX",
                                                           "1", err ) );
            v.boxSize[1] = Lexical::cast<double>( findStr( e, "boxSizeY",
                                                           "1", err ) );
            v.boxSize[2] = Lexical::cast<double>( findStr( e, "boxSizeZ",
                                                           "1", err ) );
            }

          } else
        if( tmp=="sphere" ){
          const std::string err =  "sphere diameter not declarated, set to 1.0";
          v.physModel = ProtoObject::View::Sphere;
          v.sphereDiameter = Lexical::cast<double>( findStr( e, "diameter",
                                                             "1", err ) );
          } else {
          error("invalid physic model");
          }
        } else {
        v.physModel = ProtoObject::View::NoModel;
        }

      for ( TiXmlNode* n = e->FirstChild(); n != 0; n = n->NextSibling() ){
        if ( n->Type()==TiXmlNode::TINYXML_ELEMENT ) {
          std::string name = n->Value();

          if( name == "material" ){
            std::string tmp;

            if( find( n->ToElement(), "add", tmp) ){
              v.materials.push_back( tmp );
              } else

            if( find( n->ToElement(), "del", tmp) ){              
              remove( v.materials, tmp );
              } else {
              error("invalid material definition. Use add/del operators");
              }

            }

          }
        }

      obj.view.push_back(v);
      }

    if( type=="beavior" ){
      std::string tmp;

      if( find( e, "add", tmp) ){
        obj.behaviors.push_back( tmp );
        }

      if( find( e, "del", tmp) ){
        remove( obj.behaviors, tmp );
        }
      }

    if( type=="ability" ){
      std::string tmp;

      if( find( e, "name", tmp) ){
        obj.ability.push_back( tmp );
        }
      }

    if( type=="property" ){
      readIf( e, "size", obj.data.size );
      obj.data.size = std::max(1, obj.data.size);

      readIf( e, "visionRange", obj.data.visionRange );
      obj.data.visionRange = std::max(1, obj.data.visionRange);

      readIf( e, "maxHp", obj.data.maxHp );
      obj.data.maxHp = std::max(1, obj.data.maxHp);

      readIf( e, "gold",      obj.data.gold      );
      readIf( e, "lim",       obj.data.lim       );
      readIf( e, "limInc",    obj.data.limInc    );
      readIf( e, "buildTime", obj.data.buildTime );

      readIf( e, "isBackground", obj.data.isBackground );
      readIf( e, "invincible",   obj.data.invincible   );

      std::string str;
      if( find(e, "mouseHint", str ) )
        obj.mouseHint.assign( str.begin(), str.end() );

      if( find(e, "key", str ) ){
        readIf( e, "val", obj.data.propertyStr[str] );
        }
      }

    if( type=="view_size" ){
      for( int i=0; i<3; ++i ){
        readIf( e, "min",      obj.sizeBounds.min[i] );
        readIf( e, "max",      obj.sizeBounds.max[i] );
        }

      readIf( e, "minX",      obj.sizeBounds.min[0] );
      readIf( e, "minY",      obj.sizeBounds.min[1] );
      readIf( e, "minZ",      obj.sizeBounds.min[2] );

      readIf( e, "maxX",      obj.sizeBounds.max[0] );
      readIf( e, "maxY",      obj.sizeBounds.max[1] );
      readIf( e, "maxZ",      obj.sizeBounds.max[2] );
      }

    if( type=="commands" )
      readCommands(obj, e);

    if( type=="atack" ){
      ProtoObject::GameSpecific::Atack a;
      a.damage = 0;
      a.delay  = 0;
      a.range  = 0;

      readAtack(a, e);
      obj.data.atk.push_back( a );
      }
    }

  }


void PrototypesLoader::readCommands( ProtoObject &obj, TiXmlNode *e) {
  for ( TiXmlNode* n = e->FirstChild(); n != 0; n = n->NextSibling() ){
    if ( n->Type()==TiXmlNode::TINYXML_ELEMENT ) {
      std::string s = n->Value();

      if( s=="page" ){
        readCommandsPage( obj, n->ToElement() );
        }
      }
    }
  }

void PrototypesLoader::readCommandsPage(ProtoObject &obj, TiXmlNode *node) {
  std::string str;
  TiXmlElement * e = node->ToElement();

  int p = 0;
  if( find(e, "num", str ) ){
    p = Lexical::cast<int>(str);

    if( p>=0 && size_t(p)>= obj.commands.pages.size() )
      obj.commands.pages.resize(p+1); else
      error("invalid page index");
    }

  for ( TiXmlNode* n = e->FirstChild(); n != 0; n = n->NextSibling() ){
    if ( n->Type()==TiXmlNode::TINYXML_ELEMENT ) {
      std::string s = n->Value();

      if( s=="button" ){
        ProtoObject::CmdButton btn;
        readButton( btn, n->ToElement() );
        obj.commands.pages[p].btn.push_back(btn);
        }
      }
    }
  }

void PrototypesLoader::readButton(ProtoObject::CmdButton &b, TiXmlNode *node) {
  std::string str;
  TiXmlElement * e = node->ToElement();

  b.action = ProtoObject::CmdButton::NoAction;

  if( find(e, "taget", str ) ){
    b.taget = str;
    }

  if( find(e, "icon", str ) ){
    b.icon = str;
    }

  if( find(e, "action", str ) ){
    if( str=="buy" )
      b.action = ProtoObject::CmdButton::Buy; else
    if( str=="build" )
      b.action = ProtoObject::CmdButton::Build; else
    if( str=="page" )
      b.action = ProtoObject::CmdButton::Page; else
    if( str=="castToGround" )
      b.action = ProtoObject::CmdButton::CastToGround;
    }

  if( find(e, "x", str ) )
    b.x = Lexical::cast<int>(str);  else
    b.x = 0;

  if( find(e, "y", str ) )
    b.y = Lexical::cast<int>(str);  else
    b.y = 0;

  if( find(e, "hotkey", str ) && str.size() ){
    b.hotkey = MyWidget::KeyEvent::KeyType( MyWidget::KeyEvent::K_A + str[0] - 'a' );
    }

  if( find(e, "hint", str ) && str.size() ){
    b.hint.assign( str.begin(), str.end() );
    }
  }

void PrototypesLoader::readAtack( ProtoObject::GameSpecific::Atack &b,
                                  TiXmlNode *node ) {
  std::string str;
  TiXmlElement * e = node->ToElement();

  if( find(e, "damage", str ) ){
    b.damage = Lexical::cast<int>(str);
    }
  if( find(e, "range",  str ) ){
    b.range  = Lexical::cast<int>(str);
    }
  if( find(e, "delay", str ) ){
    b.delay  = Lexical::cast<int>(str);
    }

  if( find(e, "bullet", str ) ){
    b.bullet = str;
    } else {
    b.bullet = "bullets/pike";
    }

  }

void PrototypesLoader::readSpellMember( Spell &obj, TiXmlNode *node) {
  if ( node->Type()!=TiXmlNode::TINYXML_ELEMENT )
    return;

  std::string type = node->Value();

  if( type=="property" ){
    std::string str;
    TiXmlElement * e = node->ToElement();

    if( find(e, "coolDown", str ) ){
      obj.coolDown = Lexical::cast<int>(str);
      }
    if( find(e, "manaCost",  str ) ){
      obj.manaCost  = Lexical::cast<int>(str);
      }
    }

  if( type=="info" ){
    std::string str;
    TiXmlElement * e = node->ToElement();

    if( find(e, "type", str ) ){
      if( str=="castToUnit")
        obj.mode = Spell::CastToUnit;
      if( str=="castToCoord")
        obj.mode = Spell::CastToCoord;
      }
    }
  }

bool PrototypesLoader::cmp( const std::shared_ptr<ProtoObject> &a,
                            const std::shared_ptr<ProtoObject> &b ) {
  return (a->name < b->name );
  }
