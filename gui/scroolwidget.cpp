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

void ScroolWidget::setScroolBarVisible(bool v) {
  if( v==sb.isVisible() )
    return;

  sb.setVisible(v);
  resizeEv( w(), h() );
  }

void ScroolWidget::setOrientation(Tempest::Orientation ori) {
  mlay = new ProxyLayout(ori);
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

void ScroolWidget::scrool(int ) {
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
  :LinearLayout(ori), scroolAfterEnd(1){

  }

void ScroolWidget::ProxyLayout::applyLayout() {
  int sw = widgets().size()*spacing(),
      sh = sw;

  for( size_t i=0; i<widgets().size(); ++i ){
    Tempest::Size s = sizeHint( widgets()[i] );

    sw += s.w;
    sh += s.h;
    }

  Tempest::Size sback;
  if( widgets().size() )
    sback = sizeHint( widgets().back() );

  if( orientation()==Tempest::Vertical ){
    if( scroolAfterEnd )
      scrool->setRange(0, sh-std::min( sback.h, scrool->h()) ); else
      scrool->setRange(0, sh-scrool->h() );
    owner()->setPosition( 0, -scrool->value() );
    owner()->resize( owner()->w(), sh );
    } else {
    if( scroolAfterEnd )
      scrool->setRange(0, sw-std::min(sback.w, scrool->w()));else
      scrool->setRange(0, sw-scrool->w() );
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
