#ifndef EDITTERRAINPANEL_H
#define EDITTERRAINPANEL_H

#include <MyWidget/Widget>
#include "landscape/terrain.h"

class Resource;

class EditTerrainPanel : public MyWidget::Widget {
  public:
    EditTerrainPanel( Resource & res );

    MyWidget::signal<const Terrain::EditMode&> toogleEditLandMode;

    void disableEdit(int /*unused*/);
  private:
    Terrain::EditMode m;

    void setupEHeight();
    void setupCombo();

    void setR( int r );
  };

#endif // EDITTERRAINPANEL_H
