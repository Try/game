#ifndef FONT_H
#define FONT_H

#include <string>
#include "graphics/paintergui.h"

#include <unordered_map>
#include <map>

class Resource;

namespace Tempest {

namespace Bind{
  class UserFont{
    public:
      UserFont( const std::string& name, int sz );
      UserFont( int sz );
      UserFont();

      struct Leter{
        Tempest::Bind::UserTexture surf;
        Tempest::Size  size;
        Tempest::Point dpos, advance;
        };

      const Leter& leter( Resource &res, wchar_t ch ) const;

      void fetch( Resource &res, const std::wstring& str ) const;
      Size textSize( Resource &res,
                     const std::wstring& );

      int  size() const;

      void setBold( bool b );
      bool isBold() const;

      void setItalic( bool b );
      bool isItalic() const;

      void setSize( int s );
    private:
      typedef std::unordered_map< wchar_t, Leter > Leters;
      Leters * lt;

      struct FreeTypeLib;
      static FreeTypeLib& ft();

      struct Key{
        std::string name, baseName;
        int size;
        bool bold, italic;

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

      void update();
    };
  }

}
typedef Tempest::Bind::UserFont Font;

#endif // FONT_H
