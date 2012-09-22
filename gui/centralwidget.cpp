#include "centralwidget.h"

CentralWidget::CentralWidget() {
  setLayout( new StackLayout() );
  }

MyWidget::Widget &CentralWidget::top() {
  return *layout().widgets().back();
  }

void CentralWidget::StackLayout::applyLayout() {
  for( size_t i=0; i<widgets().size(); ++i )
    placeIn( widgets()[i], MyWidget::Rect(0,0, owner()->w(), owner()->h()) );
  }
