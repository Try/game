#ifndef ABSTRACTLISTBOX_H
#define ABSTRACTLISTBOX_H

#include "gui/button.h"

class Resource;
class MainGui;
class OverlayWidget;

class AbstractListBox : public Button {
  public:
    AbstractListBox( Resource & r );

  private:
    void showList();
    void rmOverlay(Widget *);

  protected:
    Resource &res;
    OverlayWidget * overlay;

    virtual MyWidget::Widget *createDropList();
    void close();
  };

#endif // ABSTRACTLISTBOX_H
