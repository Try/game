#include "scroolwidget.h"

#include <Tempest/Layout>

ScroolWidget::ScroolWidget(Resource &res)
  :Tempest::Widget(), sb(res) {
  setLayout( Tempest::Horizontal );
  layout().add(&box);
  layout().add(&sb);

  layout().setSpacing(0);

  cen = new Widget();
  box.layout().add(cen);

  lay = new ProxyLayout();
  lay->scrool = &sb;
  cen->setLayout( lay );
  box.setLayout( Tempest::Vertical );

  onResize.bind(*this, &ScroolWidget::resizeEv);
  sb.valueChanged.bind( *this, &ScroolWidget::scrool );
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

void ScroolWidget::mouseWheelEvent(Tempest::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) || !sb.isVisible() ){
    e.ignore();
    return;
    }

  sb.setValue(sb.value() - e.delta);//*std::max(1, int(sb.range()*0.05)) );
  }

void ScroolWidget::scrool(int ) {
  lay->applyLayout();
  }

void ScroolWidget::resizeEv(int , int ) {
  int s = sb.sizePolicy().minSize.w;
  if( !sb.isVisible() )
    s = 0;

  sb.setGeometry( w()-s, 0,
                  s, h());

  box.setGeometry(0,0, w()-s, h());
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
    scrool->setRange(0, sh-std::min( sback.h, scrool->h()) );
    owner()->setPosition( 0, -scrool->value() );
    owner()->resize( owner()->w(), sh );
    } else {
    scrool->setRange(0, sw-std::min(sback.w, scrool->w()));
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
