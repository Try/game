#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <MyWidget/Widget>
#include <MyWidget/Layout>

class CentralWidget : public MyWidget::Widget {
  public:
    CentralWidget();

    MyWidget::Widget& top();
  private:
    struct StackLayout : public MyWidget::Layout {
      void applyLayout();
      };
  };

#endif // CENTRALWIDGET_H
