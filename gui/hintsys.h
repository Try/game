#ifndef HINTSYS_H
#define HINTSYS_H

#include <string>
#include <Tempest/Utility>

class HintSys {
  public:
    static void setHint( const std::wstring& current,
                         const Tempest::Rect & r );

    static const std::wstring& hint();
    static const Tempest::Point pos();

    static int time;
    static const Tempest::Rect& vrect();
  private:
    static std::wstring    current;
    static Tempest::Rect     rect;
  };

#endif // HINTSYS_H
