#ifndef LANG_H
#define LANG_H

#include <string>
#include <unordered_map>
#include <Tempest/Font>

class Resource;

class Lang {
    static std::unordered_map<std::wstring, std::wstring> lang;
    static std::wstring leters;

    template< class String >
    static std::wstring tr_impl( const String& text );
  public:
    static void load(const char* f , const wchar_t *iso3Code);

    static std::wstring tr( const std::wstring& text );
    static std::wstring tr( const std::string& text );

    static void fetch(Tempest::Font& f, Tempest::SpritesHolder& sp);
  };

#endif // LANG_H
