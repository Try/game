#include "overlaywidget.h"

#include <MyWidget/Layout>
#include "panel.h"
#include "maingui.h"

OverlayWidget::OverlayWidget( MainGui &mgui, Resource &r ):maingui(mgui) {
  setLayout( MyWidget::Vertical );
//  layout().add( new Panel(r) );

  resize(200, 200);
  setFocusPolicy( MyWidget::ClickFocus );
  setFocus(1);
  onFocusChange.bind( *this, &OverlayWidget::focusEvent );
  }

void OverlayWidget::focusEvent(bool f) {
  if( !f )
    owner()->deleteLater();
  }
