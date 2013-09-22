#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include "modalwindow.h"

class OptionsWidget : public ModalWindow {
  public:
    OptionsWidget( Resource &res, Widget* w );

  private:
    void mkControls(Resource &res, Widget* ow );
    Widget* mkPriview( Resource &res );

    template< class T >
    T* addWidget( Tempest::Widget* wx,
                  Resource& res,
                  const std::string& desc );

    void bump( int s );
    void shadow( int s );
    void physics( int p );
    void oreentation(int o);

    void applySettings();

    int shadowl, physicl;
    int bumpL, oreentationl;
  };

#endif // OPTIONSWIDGET_H
