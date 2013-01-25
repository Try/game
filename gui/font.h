#ifndef FONT_H
#define FONT_H

#include <string>
#include "graphics/paintergui.h"

#include <unordered_map>
#include <map>

class Resource;

namespace MyWidget {

namespace Bind{
  class UserFont{
    public:
      UserFont( const std::string& name, int sz );
      UserFont( int sz );
      UserFont();

      struct Leter{
        MyWidget::Bind::UserTexture surf;
        MyWidget::Size  size;
        MyWidget::Point dpos, advance;
        };

      const Leter& leter( Resource &res, wchar_t ch ) const;

      void fetch( Resource &res, const std::wstring& str ) const;
      Size textSize( Resource &res,
                     const std::wstring& );

      int size() const;
    private:
      typedef std::unordered_map< wchar_t, Leter > Leters;
      Leters * lt;

      struct FreeTypeLib;
      static FreeTypeLib& ft();

      struct Key{
        std::string name;
        int size;

        bool operator < ( const Key& other ) const{
          if( size < other.size )
            return 1;
          if( size > other.size )
            return 0;

          return name < other.name;
          }
        } key;

      static std::map<Key, Leters*> letterBox;
      const Leter& fetchLeter( Resource &res, wchar_t ch ) const;
    };
  }

}
typedef MyWidget::Bind::UserFont Font;

#endif // FONT_H
