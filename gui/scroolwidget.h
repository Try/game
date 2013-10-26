#ifndef SCROOLWIDGET_H
#define SCROOLWIDGET_H

#include <Tempest/Widget>
#include <Tempest/Layout>
#include "gui/panel.h"
#include "gui/scroolbar.h"

class ScroolWidget : public Tempest::Widget {
  public:
    ScroolWidget( Resource & res );

    Widget& centralWidget();

    void setScroolBarVisible( bool v );
    void setOrientation( Tempest::Orientation ori );
    Tempest::Orientation orientation() const;

    void scroolAfterEnd( bool s );
    bool hasScroolAfterEnd() const;
  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void gestureEvent(Tempest::AbstractGestureEvent &e);

  private:
    ScroolBar sb;
    Widget box, *cen;

    struct ProxyLayout: public Tempest::LinearLayout{
      ProxyLayout( Tempest::Orientation ori = Tempest::Vertical );
      void applyLayout();

      ScroolBar *scrool;
      bool       scroolAfterEnd;
      Tempest::Size sizeHint( const Widget *w );
      };
    ProxyLayout *mlay;

    void scrool( int v );
    void resizeEv(int w, int h);

    using Tempest::Widget::layout;
  };

#endif // SCROOLWIDGET_H
