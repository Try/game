#ifndef MAPSELECTMENU_H
#define MAPSELECTMENU_H

#include "modalwindow.h"

#include <Tempest/Timer>

class Resource;
class Button;

class MapSelectMenu : public ModalWindow {
  public:
    MapSelectMenu( Resource &res, Widget* ow );

    Tempest::signal<std::string> acepted;
  private:
    Tempest::Texture2d mPriview, base;

    void paintEvent(Tempest::PaintEvent &e);
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);

    void acceptMap();

    struct Btn;

    struct Map{
      Tempest::Texture2d mPriview;
      };
    std::vector<Map> btns;
    int  mouseDx;
    bool isAnim;

    Tempest::Point mpos, pressPos;
    Tempest::Timer timer;
    void updateT();
  };

#endif // MAPSELECTMENU_H
