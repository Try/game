#ifndef ABSTRACTXMLREADER_H
#define ABSTRACTXMLREADER_H

#include <string>
#include "util/lexicalcast.h"

class TiXmlNode;
class TiXmlElement;

class AbstractXMLReader {
  public:
    AbstractXMLReader();

    virtual void load( const std::string & s );

  protected:
    virtual void readElement( TiXmlNode *node ) = 0;
    void dumpAttribs(TiXmlElement* pElement);

    bool find( TiXmlElement* pElement,
               const std::string & key,
               std::string &ret ) const;

    std::string findStr( TiXmlElement* pElement,
                         const std::string & key,
                         const std::string & def,
                         const std::string &err );

    std::string findStrOpt( TiXmlElement* e,
                            const std::string & key,
                            const std::string & def );

    std::string findStr( TiXmlElement* pElement,
               const std::string & key,
               const std::string &err );

    template< class T >
    bool readIf( TiXmlElement* e,
                 const std::string & key,
                 T & def ){
      std::string str;
      if( find(e, key, str ) ){
        def = Lexical::cast<T>( str );
        return 1;
        }

      return 0;
      }

    void error( const std::string & what );
  private:
    struct XML;
  };

#endif // ABSTRACTXMLREADER_H
