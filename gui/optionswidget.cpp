#include "optionswidget.h"

#include <Tempest/Layout>
#include "panel.h"
#include "checkbox.h"
#include "listbox.h"

OptionsWidget::OptionsWidget( Resource &res, Widget* ow ):ModalWindow(res,ow) {
  setLayout( Tempest::Vertical );

  Widget *m = new Panel(res);

  m->setMaximumSize(400, 350);
  m->setMinimumSize(400, 350);
  m->setSizePolicy( Tempest::FixedMin );
  m->setSpacing(16);
  m->setMargin(25);
  m->setLayout( Tempest::Horizontal );

  Widget* w = new Widget();
  mkControls(res, w);
  m->layout().add(w);
  m->layout().add( mkPriview(res) );

  layout().add( new Tempest::Widget() );
  layout().add(m);
  layout().add( new Tempest::Widget() );
  }

void OptionsWidget::mkControls( Resource& res, Tempest::Widget *m) {
  m->setLayout( Tempest::Vertical );

  CheckBox *ck = new CheckBox(res);
  ck->setText("bumpMap");
  ck->setChecked( 1 );
  //ck->checked.bind( this, &GraphicsSettingsWidget::normalMap );
  m->layout().add(ck);


  ListBox *b = new ListBox(res);
  std::vector< std::wstring > l;
  //l.clear();
  l.push_back(L"512");
  l.push_back(L"1024");
  l.push_back(L"2048");

  b->setItemList(l);
  //b->setCurrentItem( s.shadowFilterQ );
  //b->onItemSelected.bind( this, &GraphicsSettingsWidget::smFilterQ );
  m->layout().add(b);
  }

Tempest::Widget *OptionsWidget::mkPriview(Resource &res) {
  Tempest::Widget *w = new Widget();
  w->setLayout( Tempest::Vertical );
  w->setMinimumSize(200,200);

  Panel *p = new Panel(res);
  p->setMinimumSize(180, 180);
  p->setSizePolicy( Tempest::FixedMin );
  w->layout().add(p);

  //Tempest::Widget *wx = new Widget();
  //wx->setLayout(Tempest::Horizontal);

  Button *b = new Button(res);
  b->clicked.bind(*this, &Widget::deleteLater );
  w->layout().add( b );
  w->layout().add( new Button(res) );

  //w->layout().add(wx);

  return w;
  }
