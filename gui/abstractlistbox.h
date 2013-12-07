#ifndef ABSTRACTLISTBOX_H
#define ABSTRACTLISTBOX_H

#include "gui/button.h"

class Resource;
class MainGui;
class OverlayWidget;

class AbstractListBox : public Button {
  public:
    AbstractListBox( Resource & r );
    ~AbstractListBox();

  private:
    void showList();
    void rmOverlay(Widget *);

  protected:
    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);

    Resource &res;
    OverlayWidget * overlay;
    bool needToShow;
    uint64_t lastRM;

    virtual Tempest::Widget *createDropList();
    void close();
  };

#endif // ABSTRACTLISTBOX_H
