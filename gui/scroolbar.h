#ifndef SCROOLBAR_H
#define SCROOLBAR_H

#include <Tempest/Widget>

#include "gui/button.h"

class Resource;
class Button;

class ScroolBar : public Tempest::Widget {
  public:
    typedef Tempest::Bind::UserTexture Texture;

    ScroolBar(Resource &res);

    void setOrientation( Tempest::Orientation ori );
    Tempest::Orientation orientation() const;

    void setRange( int min, int max );
    int range() const;
    int minValue() const;
    int maxValue() const;

    void setValue( int v );
    int  value() const;

    Tempest::signal<int> valueChanged;

  private:
    void inc();
    void dec();

    void incL();
    void decL();

    void updateValueFromView(int, unsigned);

    int rmin, rmax, smallStep, largeStep;
    int mvalue;

    Tempest::Orientation orient;

    void alignCenBtn(int, int);

    struct CenBtn: public Button {
      CenBtn( Resource & r ):Button(r) {}

      void mouseDownEvent(Tempest::MouseEvent &e);
      void mouseDragEvent(Tempest::MouseEvent &e);
      void keyPressEvent(Tempest::KeyEvent &e);

      //bool mouseTracking;
      Tempest::Point mpos, oldPos;

      void moveTo( Tempest::Point p );
      };

    struct CenWidget : public Widget {
      CenWidget( Resource & r, ScroolBar *owner );

      void mouseDownEvent(Tempest::MouseEvent &e);
      void mouseUpEvent(Tempest::MouseEvent &e);

      ScroolBar * ow;
      };

    CenWidget * cen;
    CenBtn    * cenBtn;
  };

#endif // SCROOLBAR_H
