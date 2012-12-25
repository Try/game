#include "scroolwidget.h"

#include <MyWidget/Layout>

ScroolWidget::ScroolWidget(Resource &res)
  :MyWidget::Widget(), sb(res) {
  setLayout( MyWidget::Horizontal );
  layout().add(&box);
  layout().add(&sb);

  layout().setSpacing(0);

  cen = new Widget();
  box.layout().add(cen);

  lay = new ProxyLayout();
  lay->scrool = &sb;
  cen->setLayout( lay );
  box.setLayout( MyWidget::Vertical );

  onResize.bind(*this, &ScroolWidget::resizeEv);
  sb.valueChanged.bind( *this, &ScroolWidget::scrool );
  }

MyWidget::Widget &ScroolWidget::centralWidget() {
  return *cen;
  }

void ScroolWidget::setScroolBarVisible(bool v) {
  layout().take( &sb );

  if( v )
    layout().add( &sb );
  }

void ScroolWidget::mouseWheelEvent(MyWidget::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) ){
    e.ignore();
    return;
    }

  sb.setValue( sb.value() - e.delta);//*std::max(1, int(sb.range()*0.05)) );
  }

void ScroolWidget::scrool(int v) {
  lay->applyLayout();
  }

void ScroolWidget::resizeEv(int , int ) {
  int s = sb.sizePolicy().minSize.w;
  sb.setGeometry( w()-s, 0,
                  s, h());

  box.setGeometry(0,0, w()-s, h());
  }

void ScroolWidget::ProxyLayout::applyLayout() {
  int sw = widgets().size()*spacing(),
      sh = sw;

  for( size_t i=0; i<widgets().size(); ++i ){
    MyWidget::Size s = sizeHint( widgets()[i] );

    sw += s.w;
    sh += s.h;
    }

  MyWidget::Size sback;
  if( widgets().size() )
    sback = sizeHint( widgets().back() );

  if( orientation()==MyWidget::Vertical ){
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

MyWidget::Size ScroolWidget::ProxyLayout::sizeHint(const MyWidget::Widget *wid) {
  int w = wid->sizePolicy().minSize.w,
      h = wid->sizePolicy().minSize.h;

  if( wid->sizePolicy().typeH==MyWidget::FixedMax )
    w = wid->sizePolicy().maxSize.w;
  if( wid->sizePolicy().typeV==MyWidget::FixedMax )
    h = wid->sizePolicy().maxSize.h;

  return MyWidget::Size(w,h);
  }
