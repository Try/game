#ifndef CLOSEDIALOG_H
#define CLOSEDIALOG_H

#include "modalwindow.h"

class Game;

class CloseDialog : public ModalWindow {
  public:
    CloseDialog(Game &game, Resource & res, Widget* owner );
    ~CloseDialog();

    void closeEvent(Tempest::CloseEvent &e);

    static void showCloseDialog(Tempest::CloseEvent& e, Game &game,
                                 Resource & res, Widget* owner );

  private:
    static bool shown;
    bool   pause;

    Game &game;
  };

#endif // CLOSEDIALOG_H
