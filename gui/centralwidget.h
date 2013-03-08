#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <Tempest/Widget>
#include <Tempest/Layout>

class CentralWidget : public Tempest::Widget {
  public:
    CentralWidget();

    Tempest::Widget& top();
  private:
    struct StackLayout : public Tempest::Layout {
      void applyLayout();
      };
  };

#endif // CENTRALWIDGET_H
