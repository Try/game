#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "panel.h"

class PropertyEditor : public MyWidget::Widget {
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

    MyWidget::signal< const Property&, Widget*&, EditAction > onEditor;
  protected:
    void paintEvent(MyWidget::PaintEvent &p);

    void mouseDownEvent(MyWidget::MouseEvent &e);
    void mouseUpEvent(MyWidget::MouseEvent &e);

  private:
    Resource & res;
    MyWidget::Bind::UserTexture frame, back;

    std::vector<Property> prop;

    void updateSize();
    Widget* editor;
    bool mupIntent;

    static const int rowH = 27;
  };

#endif // PROPERTYEDITOR_H
