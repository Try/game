#ifndef PROTOTYPESLOADER_H
#define PROTOTYPESLOADER_H

#include "xml/abstractxmlreader.h"

#include <unordered_map>
#include <memory>
#include "game/protoobject.h"
#include "game/spell.h"
#include "game/upgrade.h"
#include "game/particlesystemdeclaration.h"

#include <rapidjson/document.h>

class PrototypesLoader : public AbstractXMLReader {
  public:
    PrototypesLoader();
    typedef std::shared_ptr<ProtoObject> PProtoObject;
    typedef std::shared_ptr<Spell>       PSpell;
    typedef std::shared_ptr<Upgrade>     PUpgrade;

    typedef std::shared_ptr<ParticleSystemDeclaration> PParticle;

    const ProtoObject& get( const std::string & obj ) const;
    std::vector< PProtoObject> allClasses() const;

    const Spell&   spell  ( const std::string & obj ) const;
    const Upgrade& upgrade( const std::string & obj ) const;

    const ParticleSystemDeclaration& particle( const std::string & obj ) const;

    void load(const std::string &s);
    void unload();
  protected:
    void readElement(TiXmlNode *node);
    void readClassMember(ProtoObject &obj, TiXmlNode *node);
    void readCommands(ProtoObject &obj, TiXmlNode *node);
    void readCommandsPage(ProtoObject &obj, TiXmlNode *node);
    void readButton(ProtoObject::CmdButton &obj, TiXmlNode *node);
    void readAtack( ProtoObject::GameSpecific::Atack &b,
                    TiXmlNode *node );

    void readSpellMember(Spell &obj, TiXmlNode *node);
    void readGradeMember(Upgrade &obj, TiXmlNode *node);
    void readParticleMember(ParticleSystemDeclaration &obj, TiXmlNode *node);
    void readParticleMember(ParticleSystemDeclaration::D &obj, TiXmlNode *node);

    struct Defs{
      std::unordered_map<std::string, PProtoObject> data;
      std::unordered_map<std::string, PSpell>       dataSpells;
      std::unordered_map<std::string, PUpgrade>     dataUpgrades;
      std::unordered_map<std::string, PParticle>    dataParticles;
      std::vector<PSpell>   spells;
      std::vector<PUpgrade> grades;

      std::vector< std::string > unitTypes, atackTypes;
      };

    std::vector<Defs> defs;

    static bool cmp( const std::shared_ptr<ProtoObject>& a,
                     const std::shared_ptr<ProtoObject>& b );

    void loadClass( const rapidjson::Value & v );

    template< class T >
    void readIf( const rapidjson::Value & v, T& t ){
      this->readIf(v,t,t);
    }

    void readIf( const rapidjson::Value & v, int &val,  int def );
    void readIf( const rapidjson::Value & v, double &val, double def );
    void readIf( const rapidjson::Value & v, bool &val, bool def );
    void readIf( const rapidjson::Value & v, std::string &val,
                 const std::string& def );
    using AbstractXMLReader::readIf;

    void readBehavior( ProtoObject &p,
                       const rapidjson::Value &v );
    void readAtack( ProtoObject::GameSpecific::Atack &b,
                    const rapidjson::Value &v );
    void readView( ProtoObject::View &v,
                   const rapidjson::Value& e );
    void readProperty( ProtoObject &v,
                       const rapidjson::Value& e );
    void readMaterial( ProtoObject::View &v,
                       const rapidjson::Value& e );
    void readCommands( ProtoObject &obj,
                       const rapidjson::Value &v );
    void readSpell(Spell &v,
                    const rapidjson::Value& e );
  };

#endif // PROTOTYPESLOADER_H
