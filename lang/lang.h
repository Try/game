#ifndef LANG_H
#define LANG_H

#include <string>
#include <unordered_map>
#include "gui/font.h"

class Resource;

class Lang {
    static std::unordered_map<std::wstring, std::wstring> lang;
    static std::wstring leters;
  public:
    static void load( const char* f );

    static std::wstring tr( const std::wstring& text );

    static void fetch( Font& f, Resource& res );
  };

#endif // LANG_H
