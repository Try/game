#include "scroolbar.h"

#include <Tempest/Layout>
#include <iostream>

#include "gui/button.h"
#include "resource.h"

ScroolBar::ScroolBar( Resource & res ) {
  mvalue = 0;
  smallStep = 10;
  largeStep = 20;

  setRange(0, 100);

  setOrientation( Tempest::Vertical );

  Tempest::SizePolicy p;
  p.typeH = Tempest::FixedMax;
  p.typeV = Tempest::FixedMax;
  p.maxSize = Tempest::Size(27, 27);

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

void ScroolBar::setOrientation( Tempest::Orientation ori ) {
  setLayout( ori );
  layout().setMargin(0);
  orient = ori;

  Tempest::SizePolicy p;
  if( ori==Tempest::Vertical ){
    p.maxSize.w = 27;
    p.typeH = Tempest::FixedMax;
    } else {
    p.maxSize.h = 27;
    p.typeV = Tempest::FixedMax;
    }

  p.minSize = Tempest::Size(27, 27);
  setSizePolicy(p);
  }

Tempest::Orientation ScroolBar::orientation() const {
  return orient;
  }

void ScroolBar::setRange(int min, int max) {
  if( min>max )
    std::swap(min, max);

  rmin = min;
  rmax = max;
  mvalue = std::max( rmin, std::min(mvalue, rmax) );

  smallStep = std::max(1, std::min( 10, range()/100 ));
  largeStep = std::max(1, std::min( 20, range()/10 ));
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

    Tempest::Point p = cenBtn->pos();

    if( orient==Tempest::Vertical )
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
  if( orient==Tempest::Vertical )
    v = (range()*cenBtn->y())/(cen->h() - cenBtn->h()); else
    v = (range()*cenBtn->x())/(cen->w() - cenBtn->w());

  mvalue = v;
  valueChanged(v);
  }

void ScroolBar::alignCenBtn( int, int ) {
  int w = cen->w(), h = cen->h();

  if( orient==Tempest::Vertical )
    h = 40; else
    w = 40;

  cenBtn->setGeometry(0,0, w,h );
  }

void ScroolBar::CenBtn::mouseDownEvent(Tempest::MouseEvent &e) {
  Button::mouseDownEvent(e);

  mpos   = mapToRoot( e.pos() );
  oldPos = pos();
  }

void ScroolBar::CenBtn::mouseDragEvent(Tempest::MouseEvent &e) {
  moveTo( oldPos - ( mpos - mapToRoot(e.pos()) ) );
  }

void ScroolBar::CenBtn::keyPressEvent(Tempest::KeyEvent &e) {/*
  if( orient==Tempest::Vertical ){
    if( e.key==Tempest::KeyEvent::K_Up )
      moveTo( pos()+Tempest::Point(0,10) );
    }*/

  e.ignore();
  }

void ScroolBar::CenBtn::moveTo( Tempest::Point p ) {
  p.x = std::max(p.x, 0);
  p.y = std::max(p.y, 0);

  p.x = std::min( owner()->w() - w(), p.x );
  p.y = std::min( owner()->h() - h(), p.y );

  if( pos()!=p )
    setPosition( p );
  }

ScroolBar::CenWidget::CenWidget(Resource &, ScroolBar *owner) {
  ow = owner;
  }

void ScroolBar::CenWidget::mouseDownEvent(Tempest::MouseEvent &) {
  }

void ScroolBar::CenWidget::mouseUpEvent(Tempest::MouseEvent &e) {
  if( ow->orientation()==Tempest::Vertical ){
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
