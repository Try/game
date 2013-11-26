#ifndef CLOSEDIALOG_H
#define CLOSEDIALOG_H

#include "modalwindow.h"

class CloseDialog : public ModalWindow {
  public:
    CloseDialog( Resource & res, Widget* owner );
    ~CloseDialog();

    void closeEvent(Tempest::CloseEvent &e);

    static void showCloseDialog( Tempest::CloseEvent& e,
                                 Resource & res, Widget* owner );

  private:
    static bool shown;
  };

#endif // CLOSEDIALOG_H
