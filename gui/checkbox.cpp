#include "checkbox.h"

#include "resource.h"

CheckBox::CheckBox(Resource &res):Button(res) {
  state = false;
  imgCheck = res.pixmap("gui/ckBox");
  }

void CheckBox::setChecked( bool c ) {
  if( state!=c ){
    //state = c;
    emitClick();
    }
  }

bool CheckBox::isClicked() const {
  return state;
  }

Tempest::Rect CheckBox::viewRect() const {
  Tempest::Rect r = Button::viewRect();

  int dw = std::min( std::min(20,h()), r.w );
  r.w -= dw;
  r.x += dw;

  return r;
  }

void CheckBox::paintEvent( Tempest::PaintEvent &e ) {
  Tempest::Painter p(e);
  p.setBlendMode( Tempest::alphaBlend );

  int y = (h()-imgCheck.h())/2;

  Button::drawBack( p, Tempest::Rect{0,y, imgCheck.w(), imgCheck.h()} );
  Button::drawFrame(p, Tempest::Rect{0,y, imgCheck.w(), imgCheck.h()} );

  if( state ){
    p.setTexture( imgCheck );
    if( isPressed() )
      p.drawRect( 2, y+2, imgCheck.w()-4, imgCheck.h()-4,
                  0, 0, imgCheck.w(), imgCheck.h() ); else
      p.drawRect( 0, y, imgCheck.w(), imgCheck.h() );
    }

  Button::paintEvent(e);
  }

void CheckBox::emitClick() {
  state = !state;

  Button::emitClick();
  checked(state);
  }

void CheckBox::drawFrame(Tempest::Painter &) {}
void CheckBox::drawBack(Tempest::Painter &) {}
