/*
#ifndef FONT_H
#define FONT_H

#include <string>
#include "graphics/paintergui.h"

#include <Tempest/Sprite>

#include <unordered_map>
#include <map>
#include <memory>

class Resource;

namespace Tempest {

namespace Bind{
  class UserFont{
    public:
      UserFont( const std::string& name, int sz );
      UserFont( int sz );
      UserFont();

      struct Leter{
        Tempest::Sprite surf;
        Tempest::Size    size;
        Tempest::Point   dpos, advance;
        };

      const Leter& leter( Resource &res, wchar_t ch ) const;

      void fetch( Resource &res, const std::wstring& str ) const;
      void fetch( Resource &res, const std::string& str ) const;
      Size textSize( Resource &res,
                     const std::wstring& );
      Size textSize( Resource &res,
                     const std::string& );

      int  size() const;

      void setBold( bool b );
      bool isBold() const;

      void setItalic( bool b );
      bool isItalic() const;

      void setSize( int s );
    private:
      struct LMap{
        LMap(){
          l = 0;
          e = 0;
          std::fill( n, n+256, (LMap*)0 );
          }
        ~LMap(){
          delete l;
          delete e;
          for( int i=0; i<256; ++i )
            delete n[i];
          }

        Leter* find( wchar_t c ) const;
        Leter& operator[]( wchar_t c );

        private:
          mutable Leter* l;
          mutable bool * e;
          mutable LMap*  n[256];
        };

      typedef LMap Leters;
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
      const Leter& nullLeter( Resource &res, wchar_t ch ) const;

      void update();
    };
  }

}
typedef Tempest::Bind::UserFont Font;

#endif // FONT_H
*/
