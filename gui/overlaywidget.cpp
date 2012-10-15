#include "overlaywidget.h"

#include <MyWidget/Layout>
#include "panel.h"
#include "maingui.h"

OverlayWidget::OverlayWidget( MainGui &mgui, Resource &r ):maingui(mgui) {
  setLayout( MyWidget::Vertical );
//  layout().add( new Panel(r) );

  resize(200, 200);
  setFocusPolicy( MyWidget::ClickFocus );
  //setFocus(1);
  }

void OverlayWidget::setupSignals() {
  owner()->onFocusChange     .bind( *this, &OverlayWidget::focusEvent );
  owner()->onChildFocusChange.bind( *this, &OverlayWidget::focusEvent );
  }

void OverlayWidget::focusEvent(bool /*f*/) {
  if( owner() && !owner()->hasFocus() && !owner()->hasChildFocus() )
    owner()->deleteLater();
  }

void OverlayWidget::ContainerLayout::applyLayout() {
  for( size_t i=1; i<widgets().size(); ++i )
    placeIn( widgets()[i], MyWidget::Rect(0,0, owner()->w(), owner()->h()) );
  }
