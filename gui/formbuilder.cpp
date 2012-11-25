#include "formbuilder.h"

#include <MyWidget/Layout>

#include "button.h"
#include "scroolwidget.h"
#include "resource.h"

#include "propertyeditor.h"
#include "lineedit.h"

using namespace MyWidget;

template< class Base >
class FormBuilder::FormWidget:public Base{
  public:
    FormWidget( FormBuilder &owner ):Base(owner.res), owner(owner){
      setMargin(this,8);
      frame.data = owner.res.pixmap("gui/colors");

      SizePolicy p = this->sizePolicy();
      p.minSize.w = std::max( p.minSize.w, 20 );
      p.minSize.h = std::max( p.minSize.h, 20 );

      this->setSizePolicy(p);
      }

    ~FormWidget(){
      if( owner.selected==this )
        owner.selectWidget(0);
      }

    Layout& layout(){
      return layout( this );
      }

    FormBuilder & owner;

    void mouseDownEvent( MyWidget::MouseEvent &e ){
      if( owner.bintent==FormBuilder::NoIntent )
        Base::mouseDownEvent(e);
      }

    void mouseUpEvent( MyWidget::MouseEvent &e ){
      if( owner.bintent!=FormBuilder::NoIntent ){
        Widget* w = owner.createWidget();
        MyWidget::LinearLayout* l = (MyWidget::LinearLayout*)&layout();

        w->setLayout( l->orientation()==Vertical ? Horizontal:Vertical );
        layout().add( w );

        owner.bintent = NoIntent;
        } else {
        owner.selectWidget(this);
        Base::mouseUpEvent(e);
        }
      }

    void paintEvent( MyWidget::PaintEvent &e ){
      Base::paintEvent(e);
      int w = this->w(), h = this->h();

      MyWidget::Painter p(e);

      if( this==owner.selected ){
        MyWidget::Rect r   = MyWidget::Rect(0,0, w,h),
                       tex = MyWidget::Rect(1,0, 1,1);

        p.setTexture( frame );
        p.setBlendMode( MyWidget::addBlend );
        //p.drawRect( r, tex );

        r = MyWidget::Rect(r.x, r.y, 1, r.h);
        p.drawRect( r, tex );

        r = MyWidget::Rect(r.x+w-1, r.y, 1, r.h);
        p.drawRect( r, tex );

        r = MyWidget::Rect(r.x-w+2, r.y, w-2, 1);
        p.drawRect( r, tex );

        r = MyWidget::Rect(r.x, r.y+h-1, w-2, 1);
        p.drawRect( r, tex );

        p.setBlendMode( MyWidget::noBlend );
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

    void setMargin( FormWidget<ScroolWidget>* w, int m ){
      //w->layout().setMargin(m);
      }

    MyWidget::Bind::UserTexture frame;
  };

class FormBuilder::Central:public FormWidget<Panel>{
  public:
    Central( FormBuilder &owner ):FormWidget<Panel>(owner){
      setLayout( Horizontal );
      }
  };

FormBuilder::FormBuilder( Resource & res ):Panel(res), res(res) {
  selected = 0;
  bintent = NoIntent;

  resize(900, 500);
  setDragable(1);
  setLayout( Vertical );

  Widget* w = new Widget();
  layout().add( w );
  w->setLayout( Horizontal );
  w->layout().add( new Widget() );

  Button *b = new Button(res);
  b->setMaximumSize(27, 27);
  b->clicked.bind( (Widget&)*this, &FormBuilder::deleteLater );
  b->icon.data = res.pixmap("gui/ckBox");
  w->layout().add( b );

  w->setMaximumSize( w->sizePolicy().maxSize.w, 27 );
  w->setMinimumSize(0, 27);


  w = new Widget();
  w->setSizePolicy( MyWidget::Expanding );
  layout().add( w );

  w->setLayout( Horizontal );
  w->layout().add( createStdCtrlList(res) );

  Widget *wx = new Central(*this);
  wx->setSizePolicy( Expanding );
  w->layout().add( wx );
  w->layout().add( createPropertys(res) );
  }

Widget *FormBuilder::createStdCtrlList( Resource &res ) {
  ScroolWidget * w = new ScroolWidget(res);

  const char* txt[] = {
    "Button",
    "HorizontalLayout",
    "VerticalLayout",
    "Panel",
    "ScrollBox",
    "LineEdit",
    (const char*)0
    };

  void (FormBuilder::*func[])() = {
      &FormBuilder::onAddWidget<biButton>,
      &FormBuilder::onAddWidget<biPanel>,
      &FormBuilder::onAddWidget<biPanel>,
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
  prop->onEditor.bind(*this, &FormBuilder::propEditor );

  sc->centralWidget().layout().add( prop );

  p->setMinimumSize(200,0);
  p->layout().setMargin(8);
  p->setSizePolicy( FixedMin, Preferred );

  return p;
  }

void FormBuilder::setBuildable() {
  //buildable = 1;
  }

void FormBuilder::selectWidget(Widget *w) {
  selected = w;

  if( !w )
    return;

  std::vector<PropertyEditor::Property> p;
  PropertyEditor::Property tmp;

  const wchar_t *names[] = {
    L"objName",
    L"minSize",
    L"maxSize",
    0
    };

  for( int i=0; names[i]; ++i ){
    tmp.name = names[i];
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

  if( bintent==biScrollBox )
    return new FormWidget<ScroolWidget>(*this);

  if( bintent==biLineEdit )
    return new FormWidget<LineEdit>(*this);

  return new Widget();
  }

void FormBuilder::propEditor( const PropertyEditor::Property &p,
                              Widget *&w,
                              PropertyEditor::EditAction a ){

  if( a==PropertyEditor::CreateEditor )
    w = new LineEdit(res);
  }
