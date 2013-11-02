#include "button.h"

#include "resource.h"
#include "hintsys.h"

#include "sound/sound.h"
#include <Tempest/Utility>

Button::Button(Resource &res)
       : res(res), hotKey(this, Tempest::KeyEvent::K_NoKey) {
  fnt = Tempest::Font(15);

  back[0] = res.pixmap("gui/buttonBack");
  back[1] = res.pixmap("gui/buttonBackD");

  frame = res.pixmap("gui/frame");

  setFocusPolicy( Tempest::ClickFocus );

  int h = 27;
#ifdef __ANDROID__
  h = 35;
#endif

  Tempest::SizePolicy p;
  p.maxSize = Tempest::Size(128, h);
  p.minSize = Tempest::Size(27, h);
  p.typeV   = Tempest::FixedMax;
  p.typeH   = Tempest::FixedMax;

  setSizePolicy(p);

  pressed  = false;
  presAnim = false;

  onFocusChange.bind( *this, &Button::focusChange );
  timePressed = clock();
  }

void Button::setBackTexture(const Tempest::Sprite &t) {
  back[0] = t;
  back[1] = t;
  }

void Button::setShortcut(const Tempest::Shortcut &sc) {
  hotKey = sc;
  hotKey.activated.bind( this, &Button::emitClick );
  hotKey.activated.bind( this, &Button::onShortcut);
  }

const std::wstring Button::text() const {
  return txt;
  }

void Button::setText(const std::wstring &t) {
  if( txt!=t ){
    txt = t;
    //font.fetch(txt);
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

void Button::setFont(const Tempest::Font &f) {
  fnt = f;
  }

const Tempest::Font &Button::font() const {
  return fnt;
  }

void Button::mouseDownEvent(Tempest::MouseEvent &) {
  pressed  = true;
  presAnim = true;
  timePressed = clock();

  update();
  }

void Button::mouseMoveEvent( Tempest::MouseEvent & ) {
  Tempest::Point p = mapToRoot(Tempest::Point());
  HintSys::setHint( hnt, Tempest::Rect( p.x, p.y, w(), h() ));
  }

void Button::mouseUpEvent(Tempest::MouseEvent &e) {
  if( e.x <= w() && e.y <=h() &&  e.x >=0 && e.y >=0 &&
      pressed ){
    emitClick();
    res.sound("click").play();
    }

  pressed = false;
  update();
  }

void Button::mouseDragEvent(Tempest::MouseEvent &e) {
  e.ignore();
  }

void Button::paintEvent( Tempest::PaintEvent &e ) {
  Tempest::Painter p(e);
  p.setBlendMode( Tempest::alphaBlend );

  Tempest::Rect vRect = viewRect();
  Tempest::Rect r = p.scissor();

  p.setScissor( r.intersected( vRect ) );

  drawBack(p);

  if( !icon.size().isEmpty() ){
    p.setTexture( icon );

    int sz = std::min(w(), h());

    float k = std::min( sz/float(icon.w()),
                        sz/float(icon.h()) );
    k = std::min(k,1.f);

    int icW = icon.w()*k,
        icH = icon.h()*k;

    int x = std::min( ( txt.size() ? 0:(w()-icW)/2+3), (w()-icW)/2 );

    p.drawRect( x, (h()-icH)/2, icW, icH,
                0, 0, icon.w(), icon.h() );
    }

  p.setScissor(r);

  drawFrame( p );

  p.setFont(fnt);
  p.drawText( 0,0,w(),h(), txt,
              Tempest::AlignHCenter | Tempest::AlignVCenter );

  finishPaint();
  }

void Button::gestureEvent(Tempest::AbstractGestureEvent &e) {
  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture ){
    Tempest::DragGesture& g = (Tempest::DragGesture&)e;

    if( g.state()==Tempest::DragGesture::GestureUpdated ){
      pressed  = false;
      presAnim = false;
      update();
      }
    e.ignore();
    } else {
    e.ignore();
    }
  }

void Button::drawBack(Tempest::Painter &p){
  drawBack(p, viewRect());
  }

void Button::drawBack(Tempest::Painter &p, const Tempest::Rect& r ){
  const int px = r.x, py = r.y,
            pw = r.w, ph = r.h;

  Tempest::Sprite bk = back[ (hasFocus() || presAnim) ? 1:0 ];
  p.setTexture( bk );

  if( !bk.size().isEmpty() )
    p.drawRectTailed( px, py, pw, ph,
                      0, 0,
                      bk.w(), bk.h() );
  }

void Button::drawFrame( Tempest::Painter &p ) {
  drawFrame(p, viewRect());
  }

void Button::drawFrame( Tempest::Painter & p, const Tempest::Rect &vRect ) {
  int fx = 0, fy = 0;

  int px = vRect.x, py = vRect.y,
      pw = vRect.w, ph = vRect.h;

  int sz = 20;
  int bw = std::min(sz, pw/2);
  int bh = std::min(sz, ph/2);

  p.setTexture( frame );

  p.drawRect( px+bw, py, pw-2*bw, 20,
              fx+20, fy, 10, 20 );

  p.drawRect( px+bw, py+ph-20, pw-2*bw, 20,
              fx+sz, fy+frame.h()-sz, 10, 20 );


  p.drawRect( px,    py+bh, 20, ph-bh*2,
              fx,    fy+20, 20, 10 );
  p.drawRect( px+pw-20, py+bh, 20, ph-bh*2,
              fx+frame.w()-sz, fy+20, 20, 10 );


  p.drawRect( px, py, bw, bh,
              fx, fy );
  p.drawRect( px+pw-bw, py, bw, bh,
              fx+frame.w()-bw, fy );

  p.drawRect( px, py+ph-bh, bw, bh,
              fx, fy+frame.h()-bh );
  p.drawRect( px+pw-bw, py+ph-bh, bw, bh,
              fx+frame.w()-bw, fy+frame.h()-bh );
  p.unsetTexture();
  }

Tempest::Rect Button::viewRect() const {
  int px = 0, py = 0,
      pw = w(), ph = h();

  if( presAnim ){
    const int s = 1;
    px += s;
    py += s;

    pw -= 2*s;
    ph -= 2*s;
    }

  return Tempest::Rect(px, py, pw, ph);
  }

void Button::keyPressEvent(Tempest::KeyEvent &e) {
  if( false && e.key==Tempest::KeyEvent::K_F1 ){
    emitClick();
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

void Button::emitClick() {
  clicked();
  }

bool Button::isPressed() const {
  return presAnim;
  }

void Button::finishPaint() {
  if( presAnim != pressed ){
    if( clock() > timePressed+CLOCKS_PER_SEC/8 )
      presAnim = pressed;

    update();
    }
  }
