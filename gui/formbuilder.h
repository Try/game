#ifndef FORMBUILDER_H
#define FORMBUILDER_H

#include "panel.h"
#include "propertyeditor.h"

#include "modalwindow.h"

class ScroolWidget;
class Button;
class LineEdit;

class TiXmlNode;

class FormBuilder : public ModalWindow  {
  public:
    FormBuilder(Resource &res, MyWidget::Widget* ow );

  private:
    Resource &res;

    Widget *createStdCtrlList(Resource &res);
    Widget *createPropertys(Resource &res);

    enum BuildIntent{
      NoIntent,
      biButton,
      biWidget,
      biPanel,
      biScrollBox,
      biLineEdit
      } bintent;

    class Seriaziable;
    class FrmWidget;
    class Central;
    Central *centralWidget;

    template< class Base >
    class FormWidget;

    Widget* selected;
    Button* editorBtn;

    PropertyEditor *prop;
    std::string tab;

    void setBuildable();
    void selectWidget( Widget* );

    template< BuildIntent val >
    void onAddWidget();

    void setLayoutByEditor();

    Widget* createWidget();

    void propEditor( const PropertyEditor::Property& p,
                     Widget*& w,
                     PropertyEditor::EditAction a);

    void save();

    void save( Widget& w,
               std::ostream &outH,
               std::ostream &outCpp,
               std::ostream &xml,
               bool root = true );

    void save( Button& w,
               std::ostream &outH,
               std::ostream &outCpp,
               std::ostream &xml,
               bool root );

    void save( LineEdit& w,
               std::ostream &outH,
               std::ostream &outCpp,
               std::ostream &xml,
               bool root );

    void save( Panel& w,
               std::ostream &outH,
               std::ostream &outCpp,
               std::ostream &xml,
               bool root );

    void save( ScroolWidget& w,
               std::ostream &outH,
               std::ostream &outCpp,
               std::ostream &xml,
               bool root );

    void loadXML();
    void loadWidget( Widget* w, TiXmlNode * node );
    void loadWidget( Button* w, TiXmlNode * node );
    void loadWidget( LineEdit* w, TiXmlNode * node );
    void loadWidget( ScroolWidget* w, TiXmlNode * node );

    std::wstring removeEscapeSym( const std::string& str );
    };

#endif // FORMBUILDER_H
