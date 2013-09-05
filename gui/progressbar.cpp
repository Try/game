#include "progressbar.h"

#include "resource.h"
#include "font.h"

#include <Tempest/Font>

ProgressBar::ProgressBar(Resource &res)
            :hotKey(this, Tempest::KeyEvent::K_NoKey), res(res) {
 back[0] = res.pixmap("gui/hp");
 back[1] = res.pixmap("gui/gray");

 frame   = res.pixmap("gui/frame");

 setFocusPolicy( Tempest::ClickFocus );

 Tempest::SizePolicy p;
 p.maxSize = Tempest::Size(p.maxSize.w, 27);
 p.minSize = Tempest::Size(27, 27);
 p.typeV   = Tempest::FixedMax;
 p.typeH   = Tempest::Preferred;

 setSizePolicy(p);

 onFocusChange.bind( *this, &ProgressBar::focusChange );

 val  = 0;
 minV = 0;
 maxV = 100;
 }

void ProgressBar::setBackTexture(const Tempest::Sprite &t) {
 back[0] = t;
 back[1] = t;
 }

void ProgressBar::setShortcut(const Tempest::Shortcut &sc) {
 hotKey = sc;
 hotKey.activated.bind( clicked );
 }

const std::wstring ProgressBar::text() const {
 return txt;
 }

void ProgressBar::setText(const std::wstring &t) {
 if( txt!=t ){
   txt = t;
   Tempest::Font f(res.sprites());
   f.fetch(txt);
   update();
   }
 }

void ProgressBar::setText(const std::string &t) {
  std::wstring txt;
  txt.assign( t.begin(), t.end() );

  setText( txt );
  }

void ProgressBar::setValue(int v) {
  v = std::min(maxV, std::max(v, minV) );

  if( val!=v ){
    val = v;
    onValueChanged(val);
    update();
    }
  }

int ProgressBar::value() const {
  return val;
  }

void ProgressBar::setRange(int min, int max) {
  if( max<min )
    std::swap(max,min);

  if( minV==min && maxV==max )
    return;

  minV = min;
  maxV = max;

  onRangeChanged(minV, maxV);
  setValue( std::min(maxV, std::max(val, minV) ) );

  update();
  }

int ProgressBar::min() const {
  return minV;
  }

int ProgressBar::max() const {
  return maxV;
  }

void ProgressBar::mouseDownEvent(Tempest::MouseEvent &) {
  }

void ProgressBar::mouseMoveEvent(Tempest::MouseEvent &e) {
  e.accept();
  }

void ProgressBar::mouseUpEvent(Tempest::MouseEvent &e) {
  if( e.x <= w() && e.y <=h() &&  e.x >=0 && e.y >=0 )
   clicked();
  }

void ProgressBar::paintEvent( Tempest::PaintEvent &e ) {
 Tempest::Painter p(e);

 int fx = 0, fy = 0,
     px = 0, py = 0,
     pw = w(), ph = h(),
     pw2 = pw, ph2 = ph;

 int bw = std::min(20, pw2/2);
 int bh = std::min(20, ph2/2);

 if( pw2==27 )
   pw2 = 27;

 Tempest::Sprite bk = back[1];
 p.setTexture( bk );
 p.drawRect( px, py, pw2, ph2,
             0, 0,
             bk.width(), bk.height() );

 bk = back[0];
 p.setTexture( bk );

 if( maxV>minV )
   p.drawRect( px, py, pw2*val/(maxV-minV), ph2,
               0, 0,
               bk.width(), bk.height() );

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


 p.setBlendMode( Tempest::alphaBlend );
 p.setTexture( icon );
 Tempest::Font f(res.sprites());

 int sz = std::min(w(), h());

 float k = std::min( sz/float(icon.width()),
                     sz/float(icon.height()) );

 int icW = icon.width() *k,
     icH = icon.height()*k;

 p.drawRect( ( txt.size() ? 0:(w()-icW)/2), (h()-icH)/2, icW, icH,
             0, 0, icon.width(), icon.height() );

 p.setFont(f);
 p.drawText( 0,0,w(),h(), txt,
             Tempest::AlignHCenter | Tempest::AlignVCenter );
 }

void ProgressBar::keyPressEvent(Tempest::KeyEvent &e) {
 if( false && e.key==Tempest::KeyEvent::K_F1 ){
   clicked();
   } else
   e.ignore();
 }

void ProgressBar::focusChange( bool ) {
 update();
 }

