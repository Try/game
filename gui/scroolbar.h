#ifndef SCROOLBAR_H
#define SCROOLBAR_H

#include <MyWidget/Widget>

#include "gui/button.h"

class Resource;
class Button;

class ScroolBar : public MyWidget::Widget {
  public:
    typedef MyWidget::Bind::UserTexture Texture;

    ScroolBar(Resource &res);

    void setOrientation( MyWidget::Orientation ori );
    MyWidget::Orientation orientation() const;

    void setRange( int min, int max );
    int range() const;
    int minValue() const;
    int maxValue() const;

    void setValue( int v );
    int  value() const;

    MyWidget::signal<int> valueChanged;
  private:
    void inc();
    void dec();

    void incL();
    void decL();

    void updateValueFromView(int, unsigned);

    int rmin, rmax, smallStep, largeStep;
    int mvalue;

    MyWidget::Orientation orient;

    void alignCenBtn(int, int);

    struct CenBtn: public Button {
      CenBtn( Resource & r ):Button(r) {}

      void mouseDownEvent(MyWidget::MouseEvent &e);
      void mouseDragEvent(MyWidget::MouseEvent &e);
      void keyPressEvent(MyWidget::KeyEvent &e);

      //bool mouseTracking;
      MyWidget::Point mpos, oldPos;

      void moveTo( MyWidget::Point p );
      };

    struct CenWidget : public Widget {
      CenWidget( Resource & r, ScroolBar *owner );

      void mouseDownEvent(MyWidget::MouseEvent &e);
      void mouseUpEvent(MyWidget::MouseEvent &e);

      ScroolBar * ow;
      };

    CenWidget * cen;
    CenBtn    * cenBtn;
  };

#endif // SCROOLBAR_H
