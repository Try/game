#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "panel.h"

class PropertyEditor : public Tempest::Widget {
  public:
    PropertyEditor( Resource & res );

    struct Property{
      std::wstring name, valStr;
      };

    void setup( const std::vector<Property>& p );

    enum EditAction{
      CreateEditor,
      StoreEditorData,
      CloseEditor
      };

    Tempest::signal< const Property&, Widget*&, EditAction > onEditor;
  protected:
    void paintEvent(Tempest::PaintEvent &p);

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseUpEvent(Tempest::MouseEvent &e);

  private:
    Resource & res;
    Tempest::Bind::UserTexture frame, back;

    std::vector<Property> prop;

    void updateSize();
    Widget* editor;
    bool mupIntent;

    static const int rowH = 27;
  };

#endif // PROPERTYEDITOR_H
