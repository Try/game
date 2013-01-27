#include "button.h"

#include "resource.h"
#include "font.h"
#include "hintsys.h"

#include "sound/sound.h"
#include <MyWidget/Utility>

Button::Button(Resource &res)
       :hotKey(this, MyWidget::KeyEvent::K_NoKey), res(res) {
  back[0].data = res.pixmap("gui/buttonBack");
  back[1].data = res.pixmap("gui/buttonBackD");

  frame.data = res.pixmap("gui/frame");

  setFocusPolicy( MyWidget::ClickFocus );

  MyWidget::SizePolicy p;
  p.maxSize = MyWidget::Size(128, 27);
  p.minSize = MyWidget::Size(27, 27);
  p.typeV   = MyWidget::FixedMax;
  p.typeH   = MyWidget::FixedMax;

  setSizePolicy(p);

  pressed  = false;
  presAnim = false;

  onFocusChange.bind( *this, &Button::focusChange );
  timePressed = clock();
  }

void Button::setBackTexture(const Button::Texture &t) {
  back[0] = t;
  back[1] = t;
  }

void Button::setShortcut(const MyWidget::Shortcut &sc) {
  hotKey = sc;
  hotKey.activated.bind( clicked );
  hotKey.activated.bind(*this, &Button::onShortcut);
  }

const std::wstring Button::text() const {
  return txt;
  }

void Button::setText(const std::wstring &t) {
  if( txt!=t ){
    txt = t;
    Font f;
    f.fetch(res, txt);
    update();
    }
  }

void Button::setText(const std::string &t) {
  std::wstring txt;
  txt.assign( t.begin(), t.end() );

  setText( txt );
  }

void Button::setHint(const std::wstring &str) {
  hnt = str;
  }

const std::wstring &Button::hint() const {
  return hnt;
  }

void Button::mouseDownEvent(MyWidget::MouseEvent &) {
  pressed  = true;
  presAnim = true;
  timePressed = clock();

  update();
  }

void Button::mouseMoveEvent( MyWidget::MouseEvent & ) {
  MyWidget::Point p = mapToRoot(MyWidget::Point());
  HintSys::setHint( hnt, MyWidget::Rect( p.x, p.y, w(), h() ));
  }

void Button::mouseUpEvent(MyWidget::MouseEvent &e) {
  if( e.x <= w() && e.y <=h() &&  e.x >=0 && e.y >=0 ){
    clicked();
    res.sound("click").play();
    }

  pressed = false;
  update();
  }

void Button::paintEvent( MyWidget::PaintEvent &e ) {
  MyWidget::Painter p(e);

  int fx = 0, fy = 0,
      px = 0, py = 0,
      pw = w(), ph = h(),
      pw2 = pw, ph2 = ph;

  if( presAnim ){
    const int s = 1;
    px += s;
    py += s;

    pw -=s;
    ph -=s;

    pw2 = pw-s;
    ph2 = ph-s;
    }

  int bw = std::min(20, pw2/2);
  int bh = std::min(20, ph2/2);

  if( pw2==27 )
    pw2 = 27;

  Texture bk = back[ (hasFocus() || presAnim) ? 1:0 ];
  p.setTexture( bk );
  p.drawRectTailed( px, py, pw2, ph2,
                    0, 0,
                    bk.data.rect.w, bk.data.rect.h );

  p.setTexture( frame );

  p.drawRect( px+bw, py, pw-2*bw, 20,
              fx+20, fy, 10, 20 );

  p.drawRect( px+bw, ph-20, pw-2*bw, 20,
              fx+20, fy+30, 10, 20 );


  p.drawRect( px,    py+bh, 20, ph-bh*2,
              fx,    fy+20, 20, 10 );
  p.drawRect( pw-20, py+bh, 20, ph-bh*2,
              fx+30, fy+20, 20, 10 );


  p.drawRect( px, px, bw, bh,
              fx, fy );
  p.drawRect( pw-bw, py, bw, bh,
              fx+50-bw, fy );

  p.drawRect( px, ph-bh, bw, bh,
              fx, fy+50-bh );
  p.drawRect( pw-bw, ph-bh, bw, bh,
              fx+50-bw, fy+50-bh );
  p.unsetTexture();


  p.setBlendMode( MyWidget::alphaBlend );
  p.setTexture( icon );
  Font f;

  int sz = std::min(w(), h());

  float k = std::min( sz/float(icon.data.rect.w),
                      sz/float(icon.data.rect.h) );

  int icW = icon.data.rect.w*k,
      icH = icon.data.rect.h*k;

  p.drawRect( ( txt.size() ? 0:(w()-icW)/2), (h()-icH)/2, icW, icH,
              0, 0, icon.data.rect.w, icon.data.rect.h );

  p.setFont(f);
  p.drawText( 0,0,w(),h(), txt,
              MyWidget::AlignHCenter | MyWidget::AlignVCenter );

  if( presAnim != pressed ){
    if( clock() > timePressed+CLOCKS_PER_SEC/8 )
      presAnim = pressed;

    update();
    }
  }

void Button::keyPressEvent(MyWidget::KeyEvent &e) {
  if( false && e.key==MyWidget::KeyEvent::K_F1 ){
    clicked();
    presAnim = true;

    update();
    } else
    e.ignore();
  }

void Button::focusChange( bool ) {
  update();
  }

void Button::onShortcut() {
  presAnim = true;
  timePressed = clock();
  update();
  }
