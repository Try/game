#include "tabwidget.h"

#include "panel.h"
#include "button.h"

#include <MyWidget/Layout>

struct TabWidget::MPanel: public Panel{
  MPanel( Resource& res ):Panel(res){}

  void mouseDownEvent(MyWidget::MouseEvent &e){
    e.ignore();
    }
  };

struct TabWidget::MBtn: public Button {
  MBtn( Resource& res, int tab ):Button(res), tab(tab){
    clicked.bind(*this, &MBtn::exec );
    }

  MyWidget::signal<int> setPage;
  void exec(){
    setPage(tab);
    }

  int tab;
  };

TabWidget::TabWidget( Resource& res ):Panel(res), res(res) {
  setLayout( MyWidget::Vertical );
  layout().setMargin(0);

  Widget* w = new Widget();
  wbtns = w;
  w->setMaximumSize( w->sizePolicy().maxSize.w, 27 );
  w->setLayout( MyWidget::Horizontal );
  w->layout().setMargin(0);

  /*
  for( int i=0; i<3; ++i ){
    Button* b = new Button(res);
    b->setMaximumSize( 50, b->sizePolicy().maxSize.h );

    w->layout().add( b );
    }*/

  w->layout().add( new Widget() );

  p = new MPanel(res);

  layout().add( w );
  layout().add( p );
  }

TabWidget::~TabWidget() {
  for( size_t i=0; i<tabs.size(); ++i )
    delete tabs[i];
  }

void TabWidget::addTab(MyWidget::Widget *w) {
  tabs.push_back( w );

  if( tabs.size()==1 )
    setCurrentTab(0);

  if( btns.size()<tabs.size() ){
    wbtns->layout().del( wbtns->layout().widgets().back() );

    while( btns.size()<tabs.size() ){
      MBtn* b = new MBtn( res, btns.size() );
      b->setMaximumSize( 50, b->sizePolicy().maxSize.h );
      b->setPage.bind(*this, &TabWidget::setCurrentTab );

      wbtns->layout().add( b );
      btns.push_back(b);
      }

    wbtns->layout().add( new Widget() );
    }
  }

void TabWidget::setCurrentTab(int id) {
  if( id<0 && (size_t)id>=tabs.size() )
    return;

  if( p->layout().widgets().size() ){
    p->layout().take( p->layout().widgets()[0] );
    }

  p->layout().add( tabs[id] );
  onTabChanged( id );
  }
