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

    Tempest::signal<size_t> onItemSelected;
    Tempest::signal<const std::wstring&> onItemSelectedW;

    void setCurrentItem( size_t i );
  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);

    Resource &res;
  private:
    size_t selected;

    Tempest::Widget *createDropList();
    std::vector<std::wstring> data;

    class ItemBtn;

    void onItem( size_t id );
  };

#endif // LISTBOX_H
