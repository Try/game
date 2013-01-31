#ifndef PROTOTYPESLOADER_H
#define PROTOTYPESLOADER_H

#include "xml/abstractxmlreader.h"

#include <unordered_map>
#include <memory>
#include "game/protoobject.h"
#include "game/spell.h"

class PrototypesLoader : public AbstractXMLReader {
  public:
    PrototypesLoader();
    typedef std::shared_ptr<ProtoObject> PProtoObject;
    typedef std::shared_ptr<Spell>       PSpell;

    const ProtoObject& get( const std::string & obj ) const;
    std::vector< PProtoObject> allClasses() const;

    const Spell& spell( const std::string & obj ) const;
  protected:
    void readElement(TiXmlNode *node);
    void readClassMember(ProtoObject &obj, TiXmlNode *node);
    void readCommands(ProtoObject &obj, TiXmlNode *node);
    void readCommandsPage(ProtoObject &obj, TiXmlNode *node);
    void readButton(ProtoObject::CmdButton &obj, TiXmlNode *node);
    void readAtack( ProtoObject::GameSpecific::Atack &b,
                    TiXmlNode *node );


    void readSpellMember(Spell &obj, TiXmlNode *node);

    std::unordered_map<std::string, PProtoObject> data;
    std::unordered_map<std::string, PSpell> dataSpells;
    std::vector<PSpell> spells;

    static bool cmp( const std::shared_ptr<ProtoObject>& a,
                     const std::shared_ptr<ProtoObject>& b );
  };

#endif // PROTOTYPESLOADER_H
