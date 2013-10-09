#include "hintsys.h"

#include "lang/lang.h"

std::wstring    HintSys::current;
Tempest::Rect   HintSys::rect;
int             HintSys::time = 0;

void HintSys::setHint( const std::wstring &h, const Tempest::Rect &r) {
  current = Lang::tr(h);
  rect    = r;

  time    = 15;
  }

const std::wstring &HintSys::hint() {
  return current;
  }

const Tempest::Point HintSys::pos() {
  return Tempest::Point( rect.x+rect.w/2, rect.y );
  }

const Tempest::Rect &HintSys::vrect() {
  return rect;
  }
