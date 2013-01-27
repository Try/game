#include "overlaywidget.h"

#include <MyWidget/Layout>
#include "panel.h"
#include "maingui.h"


OverlayWidget::ContainerLayout::ContainerLayout() {
  used = false;
  }

OverlayWidget::OverlayWidget() {
  setLayout( new ContainerLayout() );
//  layout().add( new Panel(r) );

  resize(200, 200);
  setFocusPolicy( MyWidget::ClickFocus );
  //setFocus(1);
  }

void OverlayWidget::setupSignals() {
  onFocusChange     .bind( *this, &OverlayWidget::focusEvent );
  onChildFocusChange.bind( *this, &OverlayWidget::focusEvent );
  }

void OverlayWidget::focusEvent(bool /*f*/) {
  if( !hasFocus() && !hasChildFocus() )
    deleteLater();
  }

void OverlayWidget::mouseDownEvent(MyWidget::MouseEvent &e) {
  deleteLater();
  e.ignore();
  }

void OverlayWidget::mouseWheelEvent(MyWidget::MouseEvent &e) {
  deleteLater();
  e.ignore();
  }

void OverlayWidget::ContainerLayout::applyLayout() {
  for( size_t i=1; i<widgets().size(); ++i )
    placeIn( widgets()[i], MyWidget::Rect(0,0, owner()->w(), owner()->h()) );

  if( widgets().size()==0 ){
    if( used )
      owner()->deleteLater();
    } else {
    used = true;
    }
  }
