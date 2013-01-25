#include "hintsys.h"

#include "lang/lang.h"

std::wstring    HintSys::current;
MyWidget::Rect  HintSys::rect;
int             HintSys::time = 0;

void HintSys::setHint(const std::wstring &h, const MyWidget::Rect &r) {
  current = Lang::tr(h);
  rect    = r;

  time    = 15;
  }

const std::wstring &HintSys::hint() {
  return current;
  }

const MyWidget::Point HintSys::pos() {
  return MyWidget::Point( rect.x+rect.w/2, rect.y );
  }

const MyWidget::Rect &HintSys::vrect() {
  return rect;
  }
