#ifndef GAMEMESSAGES_H
#define GAMEMESSAGES_H

#include <Tempest/Widget>
#include <Tempest/Sprite>
#include "graphics/paintergui.h"

#include <unordered_set>

class Resource;

class GameMessages:public Tempest::Widget {
  public:
    GameMessages( Resource & res );
    ~GameMessages();

    static void message( const std::wstring &txt );
    static void message(const std::wstring& txt,
                         const Tempest::Sprite &icon );
    static void tickAll();
  protected:
    void paintEvent(Tempest::PaintEvent &e);

    void tick();
  private:
    Resource & res;

    struct MSG{
      std::wstring str;
      Tempest::Sprite icon;
      int tPrint;
      };

    static std::vector<MSG> msg;
    static std::unordered_set<GameMessages*> views;

    static void updateAll();
    static unsigned int ticksCount;

    Tempest::Sprite cride;
  };

#endif // GAMEMESSAGES_H
