#include "scroolbar.h"

#include <MyWidget/Layout>
#include <iostream>

#include "gui/button.h"
#include "resource.h"

ScroolBar::ScroolBar( Resource & res ) {
  mvalue = 0;
  smallStep = 10;
  largeStep = 20;

  setRange(0, 100);

  setOrientation( MyWidget::Vertical );

  MyWidget::SizePolicy p;
  p.typeH = MyWidget::FixedMax;
  p.typeV = MyWidget::FixedMax;
  p.maxSize = MyWidget::Size(27, 27);

  Button *b[2] = { new Button(res), new Button(res) };
  cen = new CenWidget( res, this );

  for(int i=0; i<2; ++i){
    b[i]->setSizePolicy(p);
    }

  b[0]->back[0].data = res.pixmap("gui/scroolUp");
  b[0]->back[1].data = b[0]->back[0].data;
  b[1]->back[0].data = res.pixmap("gui/scroolDown");
  b[1]->back[1].data = b[1]->back[0].data;

  b[0]->clicked.bind( *this, &ScroolBar::dec );
  b[1]->clicked.bind( *this, &ScroolBar::inc );

  layout().add( b[0] );
  layout().add( cen );
  layout().add( b[1] );

  cenBtn = new CenBtn(res);
  cenBtn->onPositionChange.bind( *this, &ScroolBar::updateValueFromView );
  cen->layout().add( cenBtn );

  onResize.bind( *this, &ScroolBar::alignCenBtn );
  alignCenBtn(-1, -1);
  }

void ScroolBar::setOrientation( MyWidget::Orientation ori ) {
  setLayout( ori );
  layout().setMargin(0);
  orient = ori;

  MyWidget::SizePolicy p;
  if( ori==MyWidget::Vertical ){
    p.maxSize.w = 27;
    p.typeH = MyWidget::FixedMax;
    } else {
    p.maxSize.h = 27;
    p.typeV = MyWidget::FixedMax;
    }

  p.minSize = MyWidget::Size(27, 27);
  setSizePolicy(p);
  }

MyWidget::Orientation ScroolBar::orientation() const {
  return orient;
  }

void ScroolBar::setRange(int min, int max) {
  if( min>max )
    std::swap(min, max);

  rmin = min;
  rmax = max;
  mvalue = std::max( rmin, std::min(mvalue, rmax) );
  }

int ScroolBar::range() const {
  return rmax-rmin;
  }

int ScroolBar::minValue() const {
  return rmin;
  }

int ScroolBar::maxValue() const {
  return rmax;
  }

void ScroolBar::setValue(int v) {
  v = std::max( rmin, std::min(v, rmax) );

  if( v!=mvalue ){
    mvalue = v;
    valueChanged(v);

    MyWidget::Point p = cenBtn->pos();

    if( orient==MyWidget::Vertical )
      p.y = v*(cen->h()-cenBtn->h())/std::max(1, rmax-rmin); else
      p.x = v*(cen->w()-cenBtn->w())/std::max(1, rmax-rmin);

    cenBtn->setPosition(p);
    }
}

int ScroolBar::value() const {
  return mvalue;
  }

void ScroolBar::inc() {
  setValue( value()+smallStep );
  }

void ScroolBar::dec() {
  setValue( value()-smallStep );
  }

void ScroolBar::incL() {
  setValue( value()+largeStep );
  }

void ScroolBar::decL() {
  setValue( value()-largeStep );
  }

void ScroolBar::updateValueFromView(int, unsigned) {
  int v;
  if( orient==MyWidget::Vertical )
    v = (range()*cenBtn->y())/(cen->h() - cenBtn->h()); else
    v = (range()*cenBtn->x())/(cen->w() - cenBtn->w());

  mvalue = v;
  valueChanged(v);
  }

void ScroolBar::alignCenBtn( int, int ) {
  int w = cen->w(), h = cen->h();

  if( orient==MyWidget::Vertical )
    h = 40; else
    w = 40;

  cenBtn->setGeometry(0,0, w,h );
  }

void ScroolBar::CenBtn::mouseDownEvent(MyWidget::MouseEvent &e) {
  Button::mouseDownEvent(e);

  mpos   = mapToRoot( e.pos() );
  oldPos = pos();
  }

void ScroolBar::CenBtn::mouseDragEvent(MyWidget::MouseEvent &e) {
  moveTo( oldPos - ( mpos - mapToRoot(e.pos()) ) );
  }

void ScroolBar::CenBtn::keyPressEvent(MyWidget::KeyEvent &e) {/*
  if( orient==MyWidget::Vertical ){
    if( e.key==MyWidget::KeyEvent::K_Up )
      moveTo( pos()+MyWidget::Point(0,10) );
    }*/

  e.ignore();
  }

void ScroolBar::CenBtn::moveTo( MyWidget::Point p ) {
  p.x = std::max(p.x, 0);
  p.y = std::max(p.y, 0);

  p.x = std::min( owner()->w() - w(), p.x );
  p.y = std::min( owner()->h() - h(), p.y );

  if( pos()!=p )
    setPosition( p );
  }

ScroolBar::CenWidget::CenWidget(Resource &r, ScroolBar *owner) {
  ow = owner;
  }

void ScroolBar::CenWidget::mouseDownEvent(MyWidget::MouseEvent &) {
  }

void ScroolBar::CenWidget::mouseUpEvent(MyWidget::MouseEvent &e) {
  if( ow->orientation()==MyWidget::Vertical ){
    if( e.pos().y < ow->cenBtn->y() )
      ow->decL();

    if( e.pos().y > ow->cenBtn->y()+ow->cenBtn->h() )
      ow->incL();
    } else {
    if( e.pos().x < ow->cenBtn->x() )
      ow->decL();

    if( e.pos().x > ow->cenBtn->x()+ow->cenBtn->w() )
      ow->incL();
    }
  }
