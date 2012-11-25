#ifndef FORMBUILDER_H
#define FORMBUILDER_H

#include "panel.h"
#include "propertyeditor.h"

class ScroolWidget;

class FormBuilder : public Panel {
  public:
    FormBuilder(Resource &res);

  private:
    Resource &res;

    Widget *createStdCtrlList(Resource &res);
    Widget *createPropertys(Resource &res);

    enum BuildIntent{
      NoIntent,
      biButton,
      biPanel,
      biScrollBox,
      biLineEdit
      } bintent;

    class Central;

    template< class Base >
    class FormWidget;

    Widget* selected;
    PropertyEditor *prop;

    void setBuildable();
    void selectWidget( Widget* );

    template< BuildIntent val >
    void onAddWidget();

    Widget* createWidget();

    void propEditor( const PropertyEditor::Property& p,
                     Widget*& w,
                     PropertyEditor::EditAction a);
  };

#endif // FORMBUILDER_H
