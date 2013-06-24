#include "formbuilder.h"

#include <fstream>
#include <Tempest/Layout>
#include <tinyxml.h>

#include "button.h"
#include "scroolwidget.h"
#include "resource.h"

#include "propertyeditor.h"
#include "lineedit.h"

#include "nativesavedialog.h"

using namespace Tempest;

class FormBuilder::FrmWidget : public Widget {
  public:
    FrmWidget( Resource & res ) {
      this->frame.data = res.pixmap("gui/colors");
      }

    void paintEvent( Tempest::PaintEvent &e ){
      Painter p(e);

      Tempest::Rect r   = Tempest::Rect(0,0, w(),h()),
                     tex = Tempest::Rect(2,0, 1,1);

      p.setTexture( frame );
      p.setBlendMode( Tempest::addBlend );
      //p.drawRect( r, tex );

      r = Tempest::Rect(r.x, r.y, 1, r.h);
      p.drawRect( r, tex );

      r = Tempest::Rect(r.x+w()-1, r.y, 1, r.h );
      p.drawRect( r, tex );

      r = Tempest::Rect(r.x-this->w()+2, r.y, this->w()-2, 1);
      p.drawRect( r, tex );

      r = Tempest::Rect(r.x, r.y+this->h()-1, this->w()-2, 1);
      p.drawRect( r, tex );

      p.unsetTexture();

      paintNested(e);
      }

  Tempest::Bind::UserTexture frame;
  };


struct FormBuilder::Seriaziable {
  std::string name;

  void setName( const std::wstring & str ){
    name.assign( str.begin(), str.end() );
    }

  virtual void save( FormBuilder & ow,
                     std::ostream &outH,
                     std::ostream &outCpp,
                     std::ostream &xml,
                     bool root ) = 0;
  };

template< class Base >
class FormBuilder::FormWidget:public Base, public Seriaziable {
  public:
    FormWidget( FormBuilder &owner ):Base(owner.res), owner(owner){
      setMargin(this,8);
      frame.data = owner.res.pixmap("gui/colors");

      SizePolicy p = this->sizePolicy();
      p.minSize.w = std::max( p.minSize.w, 20 );
      p.minSize.h = std::max( p.minSize.h, 20 );

      this->setSizePolicy(p);

      editW = 0;
      editH = 0;

      this->resize( 100, 50 );

      this->setFocusPolicy( ClickFocus );
      }

    ~FormWidget(){
      if( owner.selected==this )
        owner.selectWidget(0);
      }

    void save( FormBuilder & ow,
               std::ostream &outH,
               std::ostream &outCpp,
               std::ostream &xml,
               bool root ){
      ow.save(*this, outH, outCpp, xml, root);
      }

    Layout& layout(){
      return layout( this );
      }

    FormBuilder & owner;

    Point sdrag, oldPos;
    Size  oldSize;
    int editW, editH;


    void keyDownEvent(Tempest::KeyEvent &e){
      Base::keyDownEvent(e);

      if( dynamic_cast<Central*>(this)==0 &&
          e.key == Tempest::KeyEvent::K_Delete )
        this->deleteLater();
      }

    void mouseDownEvent( Tempest::MouseEvent &e ){
      if( owner.bintent==FormBuilder::NoIntent ){
        Layout* l = &(((Widget*)this)->owner()->layout());

        if( 0==dynamic_cast<LinearLayout*>(l) ){
          sdrag   = this->mapToRoot( e.pos() );
          oldPos  = this->pos();
          oldSize = this->size();

          editW = 0;
          editH = 0;

          int sz = 5;
          if( e.x < sz )
            editW = -1;
          if( e.x >= this->w()-sz )
            editW = 1;

          if( e.y < sz )
            editH = -1;
          if( e.y >= this->h()-sz )
            editH = 1;
          }
        }

      Base::mouseDownEvent(e);
      e.accept();
      }

    void mouseDragEvent( Tempest::MouseEvent &e ){
      Layout* l = &(((Widget*)this)->owner()->layout());

      if( 0==dynamic_cast<LinearLayout*>(l) ){
        Point dpos = this->mapToRoot( e.pos() ) - sdrag;

        if( editW ==  0 && editH ==  0 )
          this->setPosition( oldPos+dpos );

        if( editW ==  1 )
          this->setGeometry( oldPos.x, this->y(),
                             std::max(10, oldSize.w+dpos.x), this->h() );
        if( editW == -1 )
          this->setGeometry( oldPos.x+dpos.x, this->y(),
                             std::max(10, oldSize.w-dpos.x), this->h() );

        if( editH ==  1 )
          this->setGeometry( this->x(), oldPos.y,
                             this->w(), std::max(10, oldSize.h+dpos.y) );
        if( editH == -1 )
          this->setGeometry( this->x(), oldPos.y+dpos.y,
                             this->w(), std::max(10, oldSize.h-dpos.y) );
        //this->setPosition( oldPos+dpos );
        }
      }

    void mouseUpEvent( Tempest::MouseEvent &e ){
      if( owner.bintent!=FormBuilder::NoIntent ){
        Widget* w = owner.createWidget();
        w->setPosition( e.pos() );
        Tempest::LinearLayout* l = (Tempest::LinearLayout*)&layout();

        w->setLayout( l->orientation()==Vertical ? Horizontal:Vertical );
        layout().add( w );

        owner.bintent = NoIntent;
        } else {
        owner.selectWidget(this);
        Base::mouseUpEvent(e);
        e.accept();
        }

      this->update();
      }

    void paintEvent( Tempest::PaintEvent &e ){
      Base::paintEvent(e);
      int w = this->w(), h = this->h();

      Tempest::Painter p(e);

      if( this==owner.selected ){
        Tempest::Rect r   = Tempest::Rect(0,0, w,h),
                       tex = Tempest::Rect(2,0, 1,1);

        p.setTexture( frame );
        p.setBlendMode( Tempest::addBlend );
        //p.drawRect( r, tex );

        r = Tempest::Rect(r.x, r.y, 1, r.h);
        p.drawRect( r, tex );

        r = Tempest::Rect(r.x+w-1, r.y, 1, r.h);
        p.drawRect( r, tex );

        r = Tempest::Rect(r.x-w+2, r.y, w-2, 1);
        p.drawRect( r, tex );

        r = Tempest::Rect(r.x, r.y+h-1, w-2, 1);
        p.drawRect( r, tex );

        p.setBlendMode( Tempest::noBlend );
        Rect s = p.scissor();
        p.setScissor(-5, -5, w+10, h+10 );

        p.drawRect( Rect(-2,-2,5,5), tex );
        p.drawRect( Rect(-2+w,-2,5,5), tex );
        p.drawRect( Rect(-2+w,-2+h,5,5), tex );
        p.drawRect( Rect(-2,-2+h,5,5), tex );

        p.drawRect( Rect(-2+w/2,-2,5,5), tex );
        p.drawRect( Rect(-2+w/2,-2+h,5,5), tex );
        p.drawRect( Rect(-2,-2+h/2,5,5), tex );
        p.drawRect( Rect(-2+w,-2+h/2,5,5), tex );

        p.setScissor(s);
        p.unsetTexture();
        }

      }

  private:
    Layout& layout( Widget* w ){
      return w->layout();
      }

    Layout& layout( FormWidget<ScroolWidget>* w ){
      return w->centralWidget().layout();
      }

    void setMargin( Widget* w, int m ){
      w->layout().setMargin(m);
      }

    void setMargin( FormWidget<ScroolWidget>* , int m ){
      (void)m;
      //w->layout().setMargin(m);
      }

    Tempest::Bind::UserTexture frame;
  };

class FormBuilder::Central:public FormWidget<Panel>{
  public:
    Central( FormBuilder &owner ):FormWidget<Panel>(owner){
      setLayout( Horizontal );
      }
  };

FormBuilder:: FormBuilder(Resource & res , Widget *ow)
            : ModalWindow(res, ow), res(res) {
  selected  = 0;
  editorBtn = 0;
  bintent = NoIntent;

  resize( ow->w(), ow->h() );
  //setDragable(1);
  setLayout( Vertical );

  Widget* w = new Widget();
  layout().add( w );

  Button *b = new Button(res);
  b->clicked.bind( *this, (void (FormBuilder::*)() )&FormBuilder::save );
  b->setText(L"Save");
  w->layout().add( b );

  b = new Button(res);
  b->clicked.bind( *this, (void (FormBuilder::*)() )&FormBuilder::loadXML );
  b->setText(L"Load");
  w->layout().add( b );

  w->layout().add( new LineEdit(res) );

  w->setLayout( Horizontal );
  w->layout().add( new Widget() );

  b = new Button(res);
  b->setMaximumSize(27, 27);
  b->clicked.bind( (Widget&)*this, &FormBuilder::deleteLater );
  b->icon.data = res.pixmap("gui/ckBox");
  w->layout().add( b );

  w->setMaximumSize( w->sizePolicy().maxSize.w, 27 );
  w->setMinimumSize(0, 27);


  w = new Widget();
  w->setSizePolicy( Tempest::Expanding );
  layout().add( w );

  w->setLayout( Horizontal );
  w->layout().add( createStdCtrlList(res) );

  centralWidget = new Central(*this);
  centralWidget->setSizePolicy( Expanding );
  w->layout().add( centralWidget );
  w->layout().add( createPropertys(res) );
  }

Widget *FormBuilder::createStdCtrlList( Resource &res ) {
  ScroolWidget * w = new ScroolWidget(res);

  const char* txt[] = {
    "Button",
    "Widget",
    "Panel",
    "ScrollBox",
    "LineEdit",
    (const char*)0
    };

  void (FormBuilder::*func[])() = {
      &FormBuilder::onAddWidget<biButton>,
      &FormBuilder::onAddWidget<biWidget>,
      &FormBuilder::onAddWidget<biPanel>,
      &FormBuilder::onAddWidget<biScrollBox>,
      &FormBuilder::onAddWidget<biLineEdit>,
    0
    };

  for( int i=0; txt[i]; ++i ){
    Button *b = new Button(res);
    b->setText( txt[i] );
    b->clicked.bind( *this, func[i] );
    w->centralWidget().layout().add( b );
    }

  Panel *p = new Panel(res);
  p->setLayout(Horizontal);
  p->layout().add( w );
  p->setMinimumSize(180, p->sizePolicy().maxSize.h);
  p->layout().setMargin(8);

  return p;
  }

Widget *FormBuilder::createPropertys(Resource &res) {
  Panel *p = new Panel(res);
  p->setLayout(Horizontal);

  ScroolWidget *sc = new ScroolWidget(res);
  p->layout().add( sc );
  prop = new PropertyEditor(res);
  prop->onEditor.bind( this, &FormBuilder::propEditor );

  sc->centralWidget().layout().add( prop );

  p->setMinimumSize(200,0);
  p->layout().setMargin(8);
  p->setSizePolicy( FixedMin, Preferred );

  return p;
  }

void FormBuilder::setBuildable() {
  //buildable = 1;
  }

void FormBuilder::setLayoutByEditor() {
  if( selected==0 )
    return;

  if( editorBtn->text()==L"Vertical"){
    editorBtn->setText(L"Horizontal");
    selected->setLayout( Horizontal );
    } else

  if( editorBtn->text()==L"Horizontal"){
    editorBtn->setText(L"No layout");
    selected->setLayout( new Layout() );
    } else

  if( editorBtn->text()==L"No layout"){
    editorBtn->setText(L"Vertical");
    selected->setLayout( Vertical );
    }
  }

void FormBuilder::selectWidget(Widget *w) {
  selected = w;

  if( !w ){
    std::vector<PropertyEditor::Property> p;
    prop->setup(p);
    return;
    }

  std::vector<PropertyEditor::Property> p;
  PropertyEditor::Property tmp;

  const wchar_t *names[] = {
    L"objName",
    L"minSize.w",
    L"minSize.h",
    L"maxSize.w",
    L"maxSize.h",
    0
    };

  for( int i=0; names[i]; ++i ){
    tmp.name = names[i];
    p.push_back(tmp);
    }

  const std::string& name = dynamic_cast<Seriaziable*>(w)->name;
  p[0].valStr.assign( name.begin(), name.end() );

  if( LineEdit *l = dynamic_cast<LineEdit*>(w) ){
    tmp.name   = L"Text";
    tmp.valStr = l->text();
    p.push_back(tmp);
    }

  if( Button *l = dynamic_cast<Button*>(w) ){
    tmp.name   = L"Text";
    tmp.valStr = l->text();
    p.push_back(tmp);
    }

  if( Panel *l = dynamic_cast<Panel*>(w) ){
    tmp.name   = L"Layout";

    if( LinearLayout *lay = dynamic_cast<LinearLayout*>(&l->layout()) ){
      tmp.valStr = lay->orientation()==Horizontal ? L"Horizontal" : L"Vertical";
      } else {
      tmp.valStr = L"No layout";
      }

    p.push_back(tmp);
    }

  if( FrmWidget *l = dynamic_cast<FrmWidget*>(w) ){
    tmp.name   = L"Layout";

    if( LinearLayout *lay = dynamic_cast<LinearLayout*>(&l->layout()) ){
      tmp.valStr = lay->orientation()==Horizontal ? L"Horizontal" : L"Vertical";
      } else {
      tmp.valStr = L"No layout";
      }

    p.push_back(tmp);
    }


  prop->setup(p);
  }

template< FormBuilder::BuildIntent val >
void FormBuilder::onAddWidget() {
  bintent = val;
  }

Widget *FormBuilder::createWidget() {
  if( bintent==biButton )
    return new FormWidget<Button>(*this);

  if( bintent==biPanel )
    return new FormWidget<Panel>(*this);

  if( bintent==biWidget )
    return new FormWidget<FrmWidget>(*this);

  if( bintent==biScrollBox )
    return new FormWidget<ScroolWidget>(*this);

  if( bintent==biLineEdit )
    return new FormWidget<LineEdit>(*this);

  return new Widget();
  }

void FormBuilder::propEditor( const PropertyEditor::Property &p,
                              Widget *&w,
                              PropertyEditor::EditAction a ){

  if( a==PropertyEditor::CreateEditor ){
    if( p.name==L"objName" ){
      LineEdit *l = new LineEdit(res);
      w = l;

      l->setText( p.valStr );

      if( Seriaziable *wl = dynamic_cast<Seriaziable*>(selected) ){
        std::wstring str;
        str.assign( wl->name.begin(), wl->name.end() );

        l->setText( str );
        l->onTextChanged.bind(*wl, &Seriaziable::setName );
        }

      }

    if( p.name==L"Text" ){
      LineEdit *l = new LineEdit(res);
      w = l;

      l->setText( p.valStr );

      if( LineEdit *wl = dynamic_cast<LineEdit*>(selected) ){
        l->setText( wl->text() );
        l->onTextChanged.bind(*wl, &LineEdit::setText );
        }

      if( Button *wb = dynamic_cast<Button*>(selected) ){
        l->setText( wb->text() );
        void (Button::*f)( const std::wstring& ) =
            static_cast<void (Button::*)( const std::wstring& )>(&Button::setText);
        l->onTextChanged.bind(*wb, f );
        }

      }

    if( p.name==L"Layout" ){
      Button *l = new Button(res);
      w = l;
      editorBtn = l;

      l->setText( p.valStr );
      l->clicked.bind( *this, &FormBuilder::setLayoutByEditor );
      }
    }

  if( a==PropertyEditor::CloseEditor ){
    editorBtn = 0;
    }
  }

void FormBuilder::save() {
  NativeSaveDialog dlg;

  std::string fname = "./out.xml";

  if( dlg.save() ){
    fname.assign( dlg.fileName().begin(),
                  dlg.fileName().end() );
    } else {
    return;
    }

  if( fname.find('.')==std::string::npos )
    fname.push_back('.');

  while( fname.size() && fname[fname.size()-1]!='.' )
    fname.resize( fname.size()-1 );

  if( fname.size() && fname[ fname.size()-1 ]=='.' )
    fname.resize( fname.size()-1 );

  std::fstream foutH( (fname+".h").data(), std::ios::out );
  std::fstream foutCpp( (fname+".cpp").data(), std::ios::out );
  std::fstream xml( (fname+".xml").data(), std::ios::out );

  foutH << "#include <Tempest/Widget>" << std::endl << std::endl
        << "class Resource;" << std::endl
        << std::endl
        << "class Button;" << std::endl
        << "class Panel;" << std::endl
        << "class LineEdit;" << std::endl
        << "class ScroolWidget;" << std::endl
        << std::endl
        << "namespace UI{ " << std::endl << std::endl
        << "class " << "Out" << " { "   << std::endl
        << "  public: "   << std::endl
        << "    void setupUi( Tempest::Widget*, Resource & res );" << std::endl;

  foutCpp << "#include \"" << fname << ".h\" " << std::endl
          << std::endl
          << "#include <Tempest/Layout>" << std::endl
          << "#include \"gui/button.h\"" << std::endl
          << "#include \"gui/panel.h\"" << std::endl
          << "#include \"gui/scroolwidget.h\"" << std::endl
          << "#include \"gui/lineedit.h\"" << std::endl
          << std::endl
          << "using namespace Tempest;" << std::endl
          << std::endl
          << "void UI::Out::setupUi( Widget* w, Resource & res ) {" << std::endl;

  xml <<"<?xml version=\"1.0\" ?>" << std::endl
      <<"<Form>" << std::endl;

  tab = "  ";
  centralWidget->save( *this, foutH, foutCpp, xml, false );

  xml <<"</Form>" << std::endl;

  foutH << "};" << std::endl
        << "} " << std::endl;

  foutCpp << "  }" << std::endl
          << std::endl;

  foutH.close();
  foutCpp.close();
  }

void FormBuilder::save( Widget &w,
                        std::ostream &foutH,
                        std::ostream &foutCpp,
                        std::ostream &xml,
                        bool root) {
  if( root ){
    //foutCpp << "{ Tempest::Widget* owner = w;" << std::endl;
    foutCpp  << tab << "w = new Tempest::Widget();" << std::endl;

    xml << tab << "<Widget>" << std::endl;

    Seriaziable* s = dynamic_cast<Seriaziable*>(&w);
    if( s->name.size() ){
      xml << tab << "<objName string = \"" << s->name
          << "\"/>" << std::endl;

      foutH << "    Tempest::Widget * " << s->name <<";" << std::endl;
      foutCpp << tab << s->name <<" = w;" << std::endl;
      }
    }

  const char* sizePolType[] = {
    "FixedMin",
    "FixedMax",
    "Preferred",
    "Expanding"
    };

  //foutCpp << "w = new Widget();" << std::endl;
  foutCpp << tab
          << "w->setSizePolicy( "
          << sizePolType[ w.sizePolicy().typeH] <<", "
          << sizePolType[ w.sizePolicy().typeV] <<" );" << std::endl;
  foutCpp << tab
          << "w->setMaximumSize( "
          << w.sizePolicy().maxSize.w <<", "
          << w.sizePolicy().maxSize.h <<" );" << std::endl;
  foutCpp << tab
          << "w->setMinimumSize( "
          << w.sizePolicy().minSize.w <<", "
          << w.sizePolicy().minSize.h <<" );" << std::endl;

  xml << tab << "<sizePolicy x = \"" << sizePolType[ w.sizePolicy().typeH]
                    <<"\" y = \"" << sizePolType[ w.sizePolicy().typeH] <<"\" />"
                    << std::endl;

  xml << tab << "<minSize x = \"" << w.sizePolicy().minSize.w
                    <<"\" y = \"" << w.sizePolicy().minSize.h <<"\" />"
                    << std::endl;
  xml << tab << "<maxSize x = \"" << w.sizePolicy().maxSize.w
                    <<"\" y = \"" << w.sizePolicy().maxSize.h <<"\" />"
                    << std::endl;

  if( dynamic_cast<ScroolWidget*>(&w)==0 ){
    if( LinearLayout* l = dynamic_cast<LinearLayout*>(&w.layout()) ) {
      if( l->orientation()==Vertical )
        foutCpp << tab << "w->setLayout( Vertical );"; else
        foutCpp << tab << "w->setLayout( Horizontal );";
      foutCpp << std::endl;
      }
    }

  if( w.owner() && (dynamic_cast<LinearLayout*>(&w.owner()->layout())==0) ) {
    foutCpp << tab
            << "w->setGeometry( "
            << w.x() <<", "
            << w.y() <<", "
            << w.w() <<", "
            << w.h()
            <<" );" << std::endl;
    xml << tab << "<geometry x = \"" << w.x()
                       <<"\" y = \"" << w.y() <<"\" />"
                       <<"\" w = \"" << w.w() <<"\" />"
                       <<"\" h = \"" << w.h() <<"\" />"
                       << std::endl;
    }

  xml << tab << "<Nested layout = \"";

  Layout* lay = &w.layout();
  if( ScroolWidget *sw = dynamic_cast<ScroolWidget*>(&w) ){
    lay = &sw->centralWidget().layout();
    foutCpp << tab << "Tempest::Widget *pw = w;" << std::endl;
    foutCpp << tab << "w = &((ScroolWidget*)w)->centralWidget();" << std::endl;
    }

  if( LinearLayout * l = dynamic_cast<LinearLayout*>(lay) ){
    xml << ((l->orientation()==Vertical)?"Vertical" : "Horizontal");
    } else {
    xml << "NoLayout";
    }

  xml << "\" >" << std::endl;

  for( size_t i=0; i<lay->widgets().size(); ++i ){
    Seriaziable * wx = dynamic_cast<Seriaziable*>(lay->widgets()[i]);
    foutCpp << tab << "{ Widget* owner = w;" << std::endl;
    //foutCpp << "w = new Widget();" << std::endl;

    tab.resize(tab.size()+2, ' ');
    wx->save( *this, foutH, foutCpp, xml, true );

    foutCpp << tab << "owner->layout().add(w);" << std::endl;
    foutCpp << tab << "w = owner;" << std::endl;
    foutCpp << tab << "} " << std::endl;
    tab.resize( tab.size()-2 );
    }

  if( dynamic_cast<ScroolWidget*>(&w) ){
    foutCpp << tab << "w = pw;" << std::endl;
    }

  xml << tab << "</Nested>" << std::endl;

  if( root ){
    xml << tab << "</Widget>" << std::endl << std::endl;
    }
  }

void FormBuilder::save( Button &w,
                        std::ostream &foutH,
                        std::ostream &foutCpp,
                        std::ostream &xml,
                        bool root ) {
  if( root ){
    xml << tab << "<Button>" << std::endl;

    xml << tab << "<text string = \"";

    for( size_t i=0; i<w.text().size(); ++i ){
      xml << "\\x" << std::hex << int(w.text()[i]) << std::dec;
      }

    xml << "\"/>" << std::endl;
    }

  foutCpp << tab << "w = new Button(res);" << std::endl;

  if( root ){
    Seriaziable* s = dynamic_cast<Seriaziable*>(&w);
    if( s->name.size() ){
      xml << tab << "<objName string = \"" << s->name
          << "\"/>" << std::endl;

      foutH << "    Button * " << s->name <<";" << std::endl;
      foutCpp << tab << s->name <<" = (Button*)w;" << std::endl;
      }
    }

  foutCpp << tab << "((Button*)w)->setText( L\"";

  for( size_t i=0; i<w.text().size(); ++i ){
    foutCpp << "\\x" << std::hex << int(w.text()[i]) << std::dec;
    }

  foutCpp <<"\" ); "<< std::endl;

  save( (Widget&)w, foutH, foutCpp, xml, false );

  if( root ){
    xml << tab << "</Button>" << std::endl << std::endl;
    }
  }

void FormBuilder::save( LineEdit &w,
                        std::ostream &foutH,
                        std::ostream &foutCpp,
                        std::ostream &xml,
                        bool root) {
  if( root ){
    xml << tab << "<LineEdit>" << std::endl;

    xml << tab << "<text string = \"";

    for( size_t i=0; i<w.text().size(); ++i ){
      xml << "\\x" << std::hex << int(w.text()[i]) << std::dec;
      }

    xml << "\"/>" << std::endl;
    }

  foutCpp << tab << "w = new LineEdit(res);" << std::endl;

  if( root ){
    Seriaziable* s = dynamic_cast<Seriaziable*>(&w);
    if( s->name.size() ){
      xml << tab << "<objName string = \"" << s->name
          << "\"/>" << std::endl;

      foutH << "    LineEdit * " << s->name <<";" << std::endl;
      foutCpp << tab << s->name <<" = (LineEdit*)w;" << std::endl;
      }
    }

  foutCpp << tab << "((LineEdit*)w)->setText( L\"";

  for( size_t i=0; i<w.text().size(); ++i ){
    foutCpp << "\\x" << std::hex << int(w.text()[i]) << std::dec;
    }

  foutCpp <<"\" ); "<< std::endl;

  save( (Widget&)w, foutH, foutCpp, xml, false );

  if( root ){
    xml << tab << "</LineEdit>" << std::endl << std::endl;
    }
  }

void FormBuilder::save( Panel &w,
                        std::ostream &foutH,
                        std::ostream &foutCpp,
                        std::ostream &xml,
                        bool root ) {
  if( root ){
    xml << tab << "<Panel>" << std::endl;
    foutCpp << tab << "w = new Panel(res);" << std::endl;

    Seriaziable* s = dynamic_cast<Seriaziable*>(&w);
    if( s->name.size() ){
      xml << tab << "<objName string = \"" << s->name
          << "\"/>" << std::endl;

      foutH << "    Panel * " << s->name <<";" << std::endl;
      foutCpp << tab << s->name <<" = (Panel*)w;" << std::endl;
      }
    }

  save( (Widget&)w, foutH, foutCpp, xml, false );

  if( root ){
    xml << tab << "</Panel>" << std::endl << std::endl;
    }
  }

void FormBuilder::save( ScroolWidget &w,
                        std::ostream &foutH,
                        std::ostream &foutCpp,
                        std::ostream &xml,
                        bool root ) {
  if( root ){
    xml << tab << "<ScroolWidget>" << std::endl;
    foutCpp << tab << "w = new ScroolWidget(res);" << std::endl;

    Seriaziable* s = dynamic_cast<Seriaziable*>(&w);
    if( s->name.size() ){
      xml << tab << "<objName string = \"" << s->name
          << "\"/>" << std::endl;

      foutH << "    ScroolWidget * " << s->name <<";" << std::endl;
      foutCpp << tab << s->name <<" = (ScroolWidget*)w;" << std::endl;
      }
    }

  save( (Widget&)w, foutH, foutCpp, xml, false );

  if( root ){
    xml << tab << "</ScroolWidget>" << std::endl << std::endl;
    }
  }

void FormBuilder::loadXML() {
  NativeSaveDialog dlg;

  std::string fname;
  if( dlg.load() ){
    fname.assign( dlg.fileName().begin(),
                  dlg.fileName().end() );
    } else {
    return;
    }

  TiXmlDocument doc( fname.data() );
  centralWidget->layout().removeAll();

  if( !doc.LoadFile() ){
    return;
    }

  if( std::string(doc.RootElement()->Value())=="Form" ){
    loadWidget(centralWidget, doc.RootElement());
    }

  }

void FormBuilder::loadWidget(Widget *w, TiXmlNode *root ) {
  const std::string sizePolType[] = {
    "FixedMin",
    "FixedMax",
    "Preferred",
    "Expanding"
    };

  SizePolicyType t[] = {
    FixedMin,
    FixedMax,
    Preferred,
    Expanding
    };

  for( TiXmlNode* node = root->FirstChild(); node;
       node = node->NextSibling() ){
    if( std::string(node->Value())=="sizePolicy" ){
      TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

      while (attr) {
        for( int i=0; i<4; ++i ){
          if( attr->Value()==sizePolType[i] ){
            if( attr->Name()==std::string("x") ){
              w->setSizePolicy( t[i], w->sizePolicy().typeV );
              } else
            if( attr->Name()==std::string("y") ){
              w->setSizePolicy( w->sizePolicy().typeH, t[i] );
              }
            }
          }

        attr = attr->Next();
        }
      }

    if( std::string(node->Value())=="minSize" ){
      TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

      int tmp = 0;
      while (attr) {
        if( attr->Name()==std::string("x") )
          if( attr->QueryIntValue(&tmp)==TIXML_SUCCESS ){
            w->setMinimumSize( tmp, w->sizePolicy().minSize.h );
            }

        if( attr->Name()==std::string("y") )
          if( attr->QueryIntValue(&tmp)==TIXML_SUCCESS ){
            w->setMinimumSize( w->sizePolicy().minSize.w, tmp );
            }

        attr = attr->Next();
        }
      }

    if( std::string(node->Value())=="maxSize" ){
      TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

      int tmp = 0;
      while (attr) {
        if( attr->Name()==std::string("x") )
          if( attr->QueryIntValue(&tmp)==TIXML_SUCCESS ){
            w->setMaximumSize( tmp, w->sizePolicy().maxSize.h );
            }

        if( attr->Name()==std::string("y") )
          if( attr->QueryIntValue(&tmp)==TIXML_SUCCESS ){
            w->setMaximumSize( w->sizePolicy().maxSize.w, tmp );
            }

        attr = attr->Next();
        }
      }

    if( std::string(node->Value())=="objName" ){
      TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

      while (attr) {
        if( std::string(attr->Name())=="string" ){
          Seriaziable* s = dynamic_cast<Seriaziable*>(w);
          s->setName( removeEscapeSym( attr->Value() ) );
          }

        attr = attr->Next();
        }
      }

    if( std::string(node->Value())=="Nested" ){
      TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

      while (attr) {
        if( attr->Name()==std::string("layout") ){
          if( std::string(attr->Value())=="Vertical" ){
            w->setLayout( Vertical );
            } else
          if( std::string(attr->Value())=="Horizontal" ){
            w->setLayout( Horizontal );
            } else {
            w->setLayout( new Layout() );
            }
          }
        attr = attr->Next();
        }

      TiXmlNode* owner = node;

      if( ScroolWidget *sw = dynamic_cast<ScroolWidget*>(w) )
        w = &sw->centralWidget();

      for( TiXmlNode* node = owner->FirstChild(); node;
           node = node->NextSibling() ){
        if( std::string(node->Value())=="Button" ){
          Button * b = new FormWidget<Button>(*this);
          loadWidget(b, node);
          w->layout().add(b);
          }

        if( std::string(node->Value())=="Widget" ){
          FrmWidget * b = new FormWidget<FrmWidget>(*this);
          loadWidget(b, node);
          w->layout().add(b);
          }

        if( std::string(node->Value())=="Panel" ){
          Panel * b = new FormWidget<Panel>(*this);
          loadWidget(b, node);
          w->layout().add(b);
          }

        if( std::string(node->Value())=="LineEdit" ){
          LineEdit * b = new FormWidget<LineEdit>(*this);
          loadWidget(b, node);
          w->layout().add(b);
          }

        if( std::string(node->Value())=="ScroolWidget" ){
          ScroolWidget * b = new FormWidget<ScroolWidget>(*this);
          loadWidget(b, node);
          w->layout().add(b);
          }
        }

      }
    }

  }

void FormBuilder::loadWidget(Button *w, TiXmlNode *root ) {
  for( TiXmlNode* node = root->FirstChild(); node;
       node = node->NextSibling() ){
    if( std::string(node->Value())=="text" ){
      TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

      while (attr) {
        if( std::string(attr->Name())=="string" ){
          w->setText( removeEscapeSym( attr->Value() ) );
          }

        attr = attr->Next();
        }
      }
    }

  loadWidget( (Widget*)w, root );
  }

void FormBuilder::loadWidget(LineEdit *w, TiXmlNode *root ) {
  for( TiXmlNode* node = root->FirstChild(); node;
       node = node->NextSibling() ){
    if( std::string(node->Value())=="text" ){
      TiXmlAttribute* attr = node->ToElement()->FirstAttribute();

      while (attr) {
        if( std::string(attr->Name())=="string" ){
          w->setText( removeEscapeSym( attr->Value() ) );
          }

        attr = attr->Next();
        }
      }
    }

  loadWidget( (Widget*)w, root );
  }

void FormBuilder::loadWidget(ScroolWidget *w, TiXmlNode *root ) {
  loadWidget( (Widget*)w, root );
  }

std::wstring FormBuilder::removeEscapeSym(const std::string &v) {
  std::wstring ws;

  for( size_t i=0; i<v.size();  ){
    if( i+1<v.size() && v[i]=='\\' && v[i+1]=='x' ){
      i+=2;
      int ch = 0;
      while( i<v.size() &&
             (( v[i]>='0' && v[i]<='9') ||
              ( v[i]>='a' && v[i]<='f') ||
              ( v[i]>='A' && v[i]<='F') ) ){
        if( v[i]>='0' && v[i]<='9' )
          ch = ch*16 + v[i]-'0'; else

        if( v[i]>='a' && v[i]<='f' )
          ch = ch*16 + v[i]-'a'+10; else

        if( v[i]>='A' && v[i]<='F' )
          ch = ch*16 + v[i]-'A'+10;
        ++i;
        }
      ws.push_back(ch);
      } else

    if( i+1<v.size() && v[i]=='\\' && v[i+1]=='t' ){
      ws.push_back('\t');
      i += 2;
      } else

    if( i+1<v.size() && v[i]=='\\' && v[i+1]=='n' ){
      ws.push_back('\n');
      i += 2;
      } else {
      ws.push_back( v[i] );
      ++i;
      }
    }

  return ws;
  }
