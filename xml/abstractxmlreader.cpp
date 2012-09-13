#include "abstractxmlreader.h"

#include <tinyxml.h>
#include <iostream>

struct AbstractXMLReader::XML{
  static int  dump_attribs_to_stdout(TiXmlElement* pElement);
  static void readElement( AbstractXMLReader & r, TiXmlNode* pParent );

  static void parceRoot( AbstractXMLReader & r, TiXmlNode& pParent );
  };

AbstractXMLReader::AbstractXMLReader() {

  }

void AbstractXMLReader::load(const std::string &s) {
  TiXmlDocument doc( s.data() );

  if( !doc.LoadFile() ){
    error("error while load document `"+s+"'");
    std::cout << "[" << doc.ErrorRow()-1 << "]: "
              << doc.ErrorDesc() << std::endl;
    return;
    }

  XML::parceRoot( *this, doc );
  }

void AbstractXMLReader::dumpAttribs(TiXmlElement *e) {
  XML::dump_attribs_to_stdout( e );
  }

bool AbstractXMLReader::find( TiXmlElement *pElement,
                              const std::string &key,
                              std::string &ret) const {
  for( TiXmlAttribute* pAttrib=pElement->FirstAttribute();
       pAttrib;
       pAttrib=pAttrib->Next() ) {
    std::string n = pAttrib->Name();

    if( n==key ){
      ret = pAttrib->Value();
      return true;
      }
    }

  return false;
}

std::string AbstractXMLReader::findStr(TiXmlElement *e,
                                       const std::string &key,
                                       const std::string &def,
                                       const std::string &err) {
  std::string ret;
  if( find(e, key, ret ) ){
    return ret;
    } else {
    if( err.size()!=0 )
      error(err);

    return def;
  }
}

std::string AbstractXMLReader::findStr( TiXmlElement *e,
                                        const std::string &key,
                                        const std::string &err){
  return findStr( e, key, "", err );
  }

void AbstractXMLReader::error(const std::string &what) {
  std::cout << what << std::endl;
  }

void AbstractXMLReader::XML::parceRoot( AbstractXMLReader & r, TiXmlNode & root ) {
  for ( TiXmlNode* dataTag = root.FirstChild(); bool(dataTag);
        dataTag = dataTag->NextSibling() ){

    if( std::string(dataTag->Value())=="data" ){
      if ( dataTag->Type()==TiXmlNode::TINYXML_ELEMENT ) {
        //printf( "Root [%s]", dataTag->Value() );
        //dump_attribs_to_stdout( dataTag->ToElement() );
        }

      for ( TiXmlNode* n = dataTag->FirstChild(); bool(n); n = n->NextSibling() ){
        readElement( r, n );
        }

      } else {
      // error
      }

    }

  }

int AbstractXMLReader::XML::dump_attribs_to_stdout(TiXmlElement* pElement) {
  if ( !pElement )
    return 0;

  TiXmlAttribute* pAttrib=pElement->FirstAttribute();
  int i=0;
  int ival;
  double dval;
  const char* pIndent="";//getIndent(indent);
  printf("\n");

  while (pAttrib) {
    printf( "%s%s: value=[%s]", pIndent, pAttrib->Name(), pAttrib->Value());

    if (pAttrib->QueryIntValue(&ival)==TIXML_SUCCESS)
      printf( " int=%d", ival);
    if (pAttrib->QueryDoubleValue(&dval)==TIXML_SUCCESS)
      printf( " d=%1.1f", dval);
    printf( "\n" );
    i++;
    pAttrib=pAttrib->Next();
    }

  return i;
  }

void AbstractXMLReader::XML::readElement( AbstractXMLReader & r, TiXmlNode* node ) {
  r.readElement(node);
  }
