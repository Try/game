#ifndef ABSTRACTXMLREADER_H
#define ABSTRACTXMLREADER_H

#include <string>

class TiXmlNode;
class TiXmlElement;

class AbstractXMLReader {
  public:
    AbstractXMLReader();

    void load( const std::string & s );

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

    std::string findStr( TiXmlElement* pElement,
               const std::string & key,
               const std::string &err );

    void error( const std::string & what );
  private:
    struct XML;
  };

#endif // ABSTRACTXMLREADER_H
