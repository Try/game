#include "listbox.h"

#include "gui/maingui.h"
#include "gui/overlaywidget.h"
#include "gui/button.h"
#include "gui/scroolwidget.h"
#include "gui/panel.h"

class ListBox::ItemBtn:public Button{
  public:
    ItemBtn( Resource &r, size_t id ):Button(r), id(id){
      clicked.bind(*this, &ItemBtn::onClick );
      }

    size_t id;
    void onClick(){
      clickedEx(id);
      }

    MyWidget::signal<size_t> clickedEx;
  };

ListBox::ListBox( Resource &r ) : AbstractListBox(r), res(r) {
  selected = 0;
  }

void ListBox::setItemList(const std::vector<std::wstring> &list) {
  data = list;

  if( data.size() )
    setText( data[0] ); else
    setText( L"" );

  selected = 0;
  onItem(0);
  }

const std::vector<std::wstring> &ListBox::items() const {
  return data;
  }

void ListBox::setCurrentItem(size_t i) {
  if( data.size()==0 )
    return;

  i = std::min( data.size()-1, i );
  selected = i;
  onItem(selected);
  }

void ListBox::mouseWheelEvent(MyWidget::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) ){
    e.ignore();
    return;
    }

  if( data.size() ){
    if( e.delta < 0 )
      selected = (selected+1)%data.size(); else
    if( e.delta > 0 )
      selected = (selected-1)%data.size();
    onItem(selected);
    }
  }

MyWidget::Widget* ListBox::createDropList() {
  Panel *box = new Panel(res);
  box->setLayout( MyWidget::Horizontal );
  box->layout().setMargin(6);
  box->setPosition( mapToRoot( MyWidget::Point(0,h()) ) );
  box->resize(170, 200);

  ScroolWidget *sw = new ScroolWidget( res );

  for( size_t i=0; i<data.size(); ++i ){
    ItemBtn * b = new ItemBtn(res, i);
    b->setText( data[i] );
    b->clickedEx.bind( *this, &ListBox::onItem );

    sw->centralWidget().layout().add( b );
    }

  box->layout().add(sw);
  return box;
  }

void ListBox::onItem(size_t id) {
  setText( data[id] );

  onItemSelected(id);
  onItemSelectedW( data[id] );
  close();
  }
