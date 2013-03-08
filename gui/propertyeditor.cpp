#include "propertyeditor.h"

#include <Tempest/Painter>
#include "resource.h"

#include "button.h"

#include <Tempest/Layout>

#include "font.h"

using namespace Tempest;

PropertyEditor::PropertyEditor( Resource & res ):Widget(), res(res), editor(0){
  setSizePolicy( Expanding, FixedMin );
  frame.data = res.pixmap("gui/colors");
  back.data  = res.pixmap("gui/buttonBack");

  mupIntent = false;

  prop.resize(20);
  updateSize();
  }

void PropertyEditor::paintEvent(Tempest::PaintEvent &pe) {
  Painter p(pe);

  Font f;
  p.setFont(f);

  p.setTexture( back );
  p.drawRectTailed( 0,0, w(), h(),
                    0,0, back.data.rect.w, back.data.rect.h );

  p.setTexture( frame );

  for( size_t i=0; i<prop.size(); ++i ){
    p.drawRect( 0, i*rowH, w(), 1,
                6,0, 1,1);
    }
  p.drawRect( 0, prop.size()*rowH, w(), 1,
              6,0, 1,1);

  p.drawRect( w()/2, 0, 1, h(),
              6,0, 1,1);
  p.drawRect( w()-1, 0, 1, h(),
              6,0, 1,1);
  p.drawRect( 0, 0, 1, h(),
              6,0, 1,1);

  for( size_t i=0; i<prop.size(); ++i ){
    p.drawText( 5, i*rowH, w()-5, rowH, prop[i].name );
    }

  paintNested(pe);
  }

void PropertyEditor::mouseDownEvent(MouseEvent &e) {
  int id = e.y/rowH;

  if( editor ){
    onEditor( prop[id], editor, CloseEditor );
    editor->deleteLater();
    editor = 0;
    }

  if( e.x > w()/2 ){
    onEditor( prop[id], editor, CreateEditor );

    if( editor ){
      layout().add(editor);

      if( editor->focusPolicy()==ClickFocus )
        editor->setFocus(1);

      editor->setGeometry( w()/2, id*rowH, w()-w()/2, rowH );

      MouseEvent ex( e.x - editor->x(), e.y - editor->y(), e.button, e.delta );
      editor->mouseDownEvent(ex);

      mupIntent = true;
      }
    } else {
    mupIntent = false;
    }
  }

void PropertyEditor::mouseUpEvent(MouseEvent &e) {
  if( editor && mupIntent ){
    MouseEvent ex( e.x - editor->x(), e.y - editor->y(), e.button, e.delta );
    editor->mouseUpEvent(ex);
    }
  mupIntent = false;

  //editor = 0;
  }

void PropertyEditor::setup(const std::vector<Property> &p) {
  if( editor ){
    onEditor( prop[0], editor, CloseEditor );
    editor->deleteLater();
    editor = 0;
    }

  prop = p;

  Font f;
  for( size_t i=0; i<prop.size(); ++i )
    f.fetch( res, prop[i].name );

  updateSize();
  }

void PropertyEditor::updateSize() {
  setMinimumSize(20, prop.size()*rowH+1 );
  setMaximumSize( sizePolicy().maxSize.w, prop.size()*rowH );
  }
