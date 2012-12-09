#ifndef LISTBOX_H
#define LISTBOX_H

#include <vector>
#include <string>

#include "abstractlistbox.h"

class Resource;

class ListBox : public AbstractListBox {
  public:
    ListBox( Resource & r );

    void setItemList( const std::vector<std::wstring> & list );
    const std::vector<std::wstring> & items() const;

    MyWidget::signal<size_t> onItemSelected;
    MyWidget::signal<const std::wstring&> onItemSelectedW;

    void setCurrentItem( size_t i );
  protected:
    void mouseWheelEvent(MyWidget::MouseEvent &e);

  private:
    Resource &res;
    int selected;

    MyWidget::Widget *createDropList();
    std::vector<std::wstring> data;

    class ItemBtn;

    void onItem( size_t id );
  };

#endif // LISTBOX_H
