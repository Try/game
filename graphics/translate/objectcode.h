#ifndef OBJECTCODE_H
#define OBJECTCODE_H

#include <vector>
#include <memory>

#include <Tempest/VertexDeclaration>

#include "shadersource.h"

class ObjectCode {
  public:
    ObjectCode();
    ObjectCode( const ObjectCode & obj );

    ObjectCode& operator = ( const ObjectCode& obj );

    std::shared_ptr< std::wstring > wrkFolder;
    std::vector< std::shared_ptr<ShaderSource> > units;
    std::vector<size_t> input, output;

    struct Connect{
      size_t port1, port1Id;
      size_t port2, port2Id;
      };
    std::vector< Connect >     connect;

    void loadFromFile(const char *f );
    void loadFromFile(const wchar_t *f );
    void load( const char* data );

    std::shared_ptr<ShaderSource> codeOf( int outPort = -1 );
    std::shared_ptr<ShaderSource> codeOf( int outPort,
                                          std::vector<std::shared_ptr<ShaderSource> > &in);

    static Tempest::Usage::UsageType vcomponent(int v);
    static ShaderSource::TextureSemantic tsFromString( const char* ts );
    static const std::string& toString( ShaderSource::TextureSemantic t );
  private:
    static const std::string texSemantic[];
  };

#endif // OBJECTCODE_H
