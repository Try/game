#ifndef TOPPANEL_H
#define TOPPANEL_H

#include <Tempest/Widget>
class Resource;
class Button;

namespace UI{
class TopPanel {
public:
  void setupUi( Tempest::Widget*, Resource & res );

  Button * fullScr;
  Button * frmEdit;
  Button * menu;
  Button * gold;
  Button * lim;
};
}

#endif // TOPPANEL_H
