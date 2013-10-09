#ifndef MAPSELECTMENU_H
#define MAPSELECTMENU_H

#include "modalwindow.h"

#include <Tempest/Timer>

class Resource;
class Button;

class MapSelectMenu : public ModalWindow {
  public:
    MapSelectMenu( Resource &res, Widget* ow );
    ~MapSelectMenu();

    Tempest::signal<std::wstring> acepted;
  private:
    Tempest::Texture2d mPriview, base;
    Tempest::Sprite    triangle;

    void paintEvent(Tempest::PaintEvent &e);
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);

    void acceptMap(const std::wstring &str);

    struct Btn;

    struct Map{
      Tempest::Texture2d mPriview;
      std::wstring       path;
      };
    std::vector<Map> btns;
    int  mouseDx;
    bool isAnim;

    Tempest::Point mpos, pressPos;
    Tempest::Timer timer;
    void updateT();

    Tempest::Rect rect( int i );
    Tempest::Rect rect( int i, const Tempest::Texture2d &t );
  };

#endif // MAPSELECTMENU_H
