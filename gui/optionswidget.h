#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include "modalwindow.h"

class OptionsWidget : public ModalWindow {
  public:
    OptionsWidget( Resource &res, Widget* w );

  private:
    void mkControls(Resource &res, Widget* ow );
    Widget* mkPriview( Resource &res );
  };

#endif // OPTIONSWIDGET_H
