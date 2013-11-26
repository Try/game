#include "scroolwidget.h"

#include <Tempest/Layout>

ScroolWidget::ScroolWidget(Resource &res)
  :Tempest::Widget(), sb(res) {
  //setLayout( Tempest::Horizontal );
  layout().add(&box);
  layout().add(&sb);

  layout().setSpacing(0);

  cen = new Widget();
  box.layout().add(cen);

  mlay = new ProxyLayout();
  mlay->scrool = &sb;
  cen->setLayout( mlay );
  box.setLayout( Tempest::Vertical );

  onResize.bind( this, &ScroolWidget::resizeEv);
  sb.valueChanged.bind( *this, &ScroolWidget::scrool );

#ifdef __ANDROID__
  sb.setVisible(0);
#endif
  }

Tempest::Widget &ScroolWidget::centralWidget() {
  return *cen;
  }

void ScroolWidget::hideScroolBar() {
  setScroolBarVisible(0);
  }

void ScroolWidget::setScroolBarVisible(bool v) {
  if( v==sb.isVisible() )
    return;

  sb.setVisible(v);
  resizeEv( w(), h() );
  }

void ScroolWidget::setOrientation(Tempest::Orientation ori) {
  mlay = new ProxyLayout(ori);
  mlay->scroolAfterEnd    = ((ProxyLayout&)cen->layout()).scroolAfterEnd;
  mlay->scroolBeforeBegin = ((ProxyLayout&)cen->layout()).scroolBeforeBegin;

  mlay->scrool = &sb;
  cen->setLayout( mlay );

  box.setLayout( ori );

  sb.setOrientation( ori );
  //setLayout( inv );
  }

Tempest::Orientation ScroolWidget::orientation() const {
  return sb.orientation();
  }

void ScroolWidget::scroolAfterEnd(bool s) {
  mlay->scroolAfterEnd = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScroolWidget::hasScroolAfterEnd() const {
  return mlay->scroolAfterEnd;
  }

void ScroolWidget::scroolBeforeBegin(bool s) {
  mlay->scroolBeforeBegin = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScroolWidget::hasScroolBeforeBegin() const {
  return mlay->scroolBeforeBegin;
  }

void ScroolWidget::mouseWheelEvent(Tempest::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) || !sb.isVisible() ){
    e.ignore();
    return;
    }

  if( orientation()==Tempest::Vertical )
    sb.setValue(sb.value() - e.delta);
  }

void ScroolWidget::mouseMoveEvent(Tempest::MouseEvent &e) {
  e.ignore();
  }

void ScroolWidget::gestureEvent(Tempest::AbstractGestureEvent &e) {
  e.ignore();
  if( sb.isVisible() )
    return;

  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture ){
    Tempest::DragGesture &d = (Tempest::DragGesture&)(e);
    int v = sb.value();
    int dpos = d.dpos.y;
    if( orientation()==Tempest::Horizontal )
      dpos = d.dpos.x;

    sb.setValue(sb.value() - dpos );
    if( v!=sb.value() )
      e.accept();
    }
  }

void ScroolWidget::scrool( int v ) {
  sb.setValue( v );
  mlay->applyLayout();
  }

void ScroolWidget::resizeEv(int , int ) {
  if( sb.isVisible() ){
    if( orientation()==Tempest::Vertical ){
      int s = sb.sizePolicy().minSize.w;
      if( !sb.isVisible() )
        s = 0;

      sb.setGeometry( w()-s, 0,
                      s, h());

      box.setGeometry(0,0, w()-s, h());
      } else {
      int s = sb.sizePolicy().minSize.h;
      if( !sb.isVisible() )
        s = 0;

      sb.setGeometry( 0, h()-s,
                      w(), s );

      box.setGeometry(0,0, w(), h()-s);
      }
    } else {
    box.setGeometry(0,0, w(), h());
    }
  }

ScroolWidget::ProxyLayout::ProxyLayout(Tempest::Orientation ori)
  :LinearLayout(ori), scroolAfterEnd(1), scroolBeforeBegin(0){

  }

void ScroolWidget::ProxyLayout::applyLayout() {
  int sw = widgets().size()*spacing(),
      sh = sw;

  for( size_t i=0; i<widgets().size(); ++i ){
    Tempest::Size s = sizeHint( widgets()[i] );

    sw += s.w;
    sh += s.h;
    }

  Widget* sbox = owner()->owner();

  Tempest::Size sback, stop;
  if( widgets().size() ){
    sback = sizeHint( widgets().back() );
    stop  = sizeHint( widgets()[0] );
    }

  if( orientation()==Tempest::Vertical ){
    int sscroll = 0;
    if( scroolBeforeBegin )
      sscroll = -std::max(sbox->h()-stop.h, stop.h);

    if( scroolAfterEnd )
      scrool->setRange( sscroll, sh-std::min( sback.h, sbox->h()) ); else
      scrool->setRange( sscroll, sh-scrool->h() );
    owner()->setPosition( 0, -scrool->value() );
    owner()->resize( owner()->w(), sh );
    } else {
    int sscroll = 0;
    if( scroolBeforeBegin )
      sscroll = -std::max(sbox->w()-stop.w, stop.w);

    if( scroolAfterEnd )
      scrool->setRange( sscroll, sw-std::min(sback.w, sbox->w()));else
      scrool->setRange( sscroll, sw-scrool->w() );
    owner()->setPosition( -scrool->value(), 0 );
    owner()->resize( sw, owner()->h());
    }

  LinearLayout::applyLayout();
  }

Tempest::Size ScroolWidget::ProxyLayout::sizeHint(const Tempest::Widget *wid) {
  int w = wid->sizePolicy().minSize.w,
      h = wid->sizePolicy().minSize.h;

  if( wid->sizePolicy().typeH==Tempest::FixedMax )
    w = wid->sizePolicy().maxSize.w;
  if( wid->sizePolicy().typeV==Tempest::FixedMax )
    h = wid->sizePolicy().maxSize.h;

  return Tempest::Size(w,h);
  }
