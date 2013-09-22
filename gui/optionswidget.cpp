#include "optionswidget.h"

#include <Tempest/Layout>
#include "graphicssettingswidget.h"
#include "panel.h"
#include "checkbox.h"
#include "listbox.h"
#include "lineedit.h"

#include "lang/lang.h"

OptionsWidget::OptionsWidget( Resource &res, Widget* ow ):ModalWindow(res,ow) {
  setLayout( Tempest::Vertical );

  Widget *m = new Panel(res);

  m->setMaximumSize(320, 300);
  m->setMinimumSize(320, 300);
  m->setSizePolicy( Tempest::FixedMin );
  m->setSpacing(16);
  m->setMargin(25);
  m->setLayout( Tempest::Horizontal );

  Widget* w = new Widget();
  mkControls(res, w);
  m->layout().add(w);
  //m->layout().add( mkPriview(res) );

  layout().add( new Tempest::Widget() );
  layout().add(m);
  layout().add( new Tempest::Widget() );
  layout().add( new Tempest::Widget() );
  }

void OptionsWidget::mkControls( Resource& res, Tempest::Widget *m) {
  m->setLayout( Tempest::Vertical );

  GraphicsSettingsWidget::Settings s = GraphicsSettingsWidget::Settings::settings();

  std::vector< std::wstring > l;
  l.push_back( Lang::tr("$(options/disable)") );
  l.push_back( Lang::tr("$(options/low)") );
  l.push_back( Lang::tr("$(options/mid)") );
  l.push_back( Lang::tr("$(options/hight)") );

  //b->setCurrentItem( s.shadowFilterQ );
  //b->onItemSelected.bind( this, &GraphicsSettingsWidget::smFilterQ );

  ListBox *lb = 0;
  lb = addWidget<ListBox> (m, res, "$(options/shadow)" );
  lb->setItemList(l);
  shadowl = 0;
  if( s.shadowMapRes==512 )
    shadowl = 1;
  if( s.shadowMapRes==1024 )
    shadowl = 2;
  if( s.shadowMapRes==2048 )
    shadowl = 3;
  lb->setCurrentItem(shadowl);

  lb->onItemSelected.bind(this, &OptionsWidget::shadow);

  bumpL = s.normalMap ? 1:0;
  CheckBox *cb = 0;
  cb = addWidget<CheckBox>(m, res, "$(options/bump)"   );
  cb->setChecked(bumpL);
  cb->checked.bind(this, &OptionsWidget::bump );

  lb = addWidget<ListBox> (m, res, "$(options/physics)");
  lb->setItemList(l);
  physicl = s.physics;
  lb->setCurrentItem(physicl);
  lb->onItemSelected.bind(this, &OptionsWidget::physics);

  std::vector< std::wstring > lo;
  lo.push_back( Lang::tr("$(options/any)") );
  lo.push_back( Lang::tr("$(options/landscape)") );
  lo.push_back( Lang::tr("$(options/portraint)") );
  lb = addWidget<ListBox> (m, res, "$(options/oreentation)");
  lb->setItemList(lo);
  oreentationl = s.oreentation;
  lb->setCurrentItem( oreentationl );
  lb->onItemSelected.bind(this, &OptionsWidget::oreentation);

  Widget* w = new Widget();
  w->setLayout( Tempest::Horizontal );

  Button* b = new Button(res);
  b->setText( Lang::tr("$(options/ok)") );
  b->clicked.bind(this, &OptionsWidget::applySettings );
  w->layout().add( b );

  b = new Button(res);
  b->setText( Lang::tr("$(options/cancel)") );
  b->clicked.bind(this, &ModalWindow::deleteLater );
  w->layout().add( b );

  m->layout().add(w);
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

void OptionsWidget::bump(int s) {
  bumpL = s;
  }

void OptionsWidget::shadow(int s) {
  shadowl = s;
  }

void OptionsWidget::physics(int p) {
  physicl = p;
  }

void OptionsWidget::oreentation(int o) {
  oreentationl = o;
  }

void OptionsWidget::applySettings() {
  GraphicsSettingsWidget::Settings s = GraphicsSettingsWidget::Settings::settings();

  int sm[] = {0, 512, 1024, 2048};
  s.shadowMapRes = sm[shadowl];
  s.normalMap    = bumpL>0;
  s.physics      = physicl;
  s.oreentation  = oreentationl;

  GraphicsSettingsWidget::Settings::setSettings(s);
  GraphicsSettingsWidget::Settings::save();

  deleteLater();
  }

template< class T >
T *OptionsWidget::addWidget( Tempest::Widget* wx,
                             Resource& res,
                             const std::string &desc) {
  Widget* w = new Widget();
  w->setLayout( Tempest::Horizontal );

  LineEdit *e = new LineEdit(res);
  e->setText( Lang::tr(desc) );
  e->setEditable(0);
  w->layout().add(e);

  T* t = new T(res);
  w->layout().add(t);

  wx->layout().add(w);

  return t;
  }
