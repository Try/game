#include "lang.h"

#include <fstream>

#include <unordered_set>

#include "util/ifstream.h"

std::unordered_map<std::wstring, std::wstring> Lang::lang;
std::wstring Lang::leters;

void Lang::load(const char *f) {
  ifstream fin( f );
  std::unordered_set<wchar_t, std::hash<size_t> > let;

  size_t c = 0;
  fin.read( (char*)&c, sizeof(c) );

  std::vector<int16_t> tmpBuf;

  for( size_t i=0; i<c; ++i ){
    std::wstring kv[2];

    for( int r=0; r<2; ++r ){
      std::wstring& str = kv[r];
      size_t sz = str.size();

      fin.read( (char*)&sz, sizeof(sz) );
      str.resize( sz );
      tmpBuf.resize( std::max(sz, tmpBuf.size()) );

      fin.read( (char*)&tmpBuf[0], sz*sizeof(int16_t) );

      str.assign( tmpBuf.begin(), tmpBuf.begin()+sz );

      for( size_t q=0; q<sz; ++q )
        let.insert(str[q]);
      }

    lang[kv[0]] = kv[1];
    }

  leters.assign( let.begin(), let.end() );
  }

template< class String >
std::wstring Lang::tr_impl(const String &text) {
  std::wstring out;

  size_t sz = text.size()-1;

  if( text.size() ){
    size_t i=0;
    for( i=0; i<sz; ++i ){
      if( text[i]=='$' && text[i+1]=='(' ){
        i+=2;
        std::wstring key;
        while( i<sz && text[i]!=')' ){
          key += text[i];
          ++i;
          }

        std::unordered_map<std::wstring, std::wstring>::iterator kv =
            lang.find(key);
        if( kv!=lang.end() ){
          out += kv->second;
          } else {
          //out += L"$(";
          out += key;
          //out += L")";
          }

        } else {
        out.push_back( text[i] );
        }

      }

    if( i<text.size() )
      out += text[i];
    }

  return out;
  }

std::wstring Lang::tr(const std::wstring &text) {
  return tr_impl(text);
  }

std::wstring Lang::tr(const std::string &text) {
  return tr_impl(text);
  }

void Lang::fetch(Tempest::Font &f , Tempest::SpritesHolder &sp) {
  f.fetch(leters, sp);
  }
