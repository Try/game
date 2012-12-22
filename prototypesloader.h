#ifndef PROTOTYPESLOADER_H
#define PROTOTYPESLOADER_H

#include "xml/abstractxmlreader.h"

#include <unordered_map>
#include <memory>
#include "game/protoobject.h"

class PrototypesLoader : public AbstractXMLReader {
  public:
    PrototypesLoader();
    typedef std::shared_ptr<ProtoObject> PProtoObject;

    const ProtoObject& get( const std::string & obj ) const;
    std::vector< PProtoObject> allClasses() const;

  protected:
    void readElement(TiXmlNode *node);
    void readClassMember(ProtoObject &obj, TiXmlNode *node);
    void readCommands(ProtoObject &obj, TiXmlNode *node);
    void readCommandsPage(ProtoObject &obj, TiXmlNode *node);
    void readButton(ProtoObject::CmdButton &obj, TiXmlNode *node);
    void readAtack( ProtoObject::GameSpecific::Atack &b,
                    TiXmlNode *node );

    std::unordered_map<std::string, PProtoObject> data;

    static bool cmp( const std::shared_ptr<ProtoObject>& a,
                     const std::shared_ptr<ProtoObject>& b );
  };

#endif // PROTOTYPESLOADER_H
