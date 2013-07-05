#ifndef EDITTERRAINPANEL_H
#define EDITTERRAINPANEL_H

#include <Tempest/Widget>
#include "landscape/terrain.h"

class Resource;

class EditTerrainPanel : public Tempest::Widget {
  public:
    EditTerrainPanel(Resource & res,
                      const PrototypesLoader &prototypes );

    Tempest::signal<const Terrain::EditMode&> toogleEditLandMode;

    void disableEdit(int /*unused*/);
  private:
    Terrain::EditMode m;

    struct TileBtn;

    void setupEHeight();
    void setupCombo();
    void setupAlign();
    void setupSmooth();
    void setupRm();

    void setTexture( const std::string& str );

    void setR( int r );

    template<int i>
    void setTexID(){
      m.isSecondaryTexturing = (i==1);
      toogleEditLandMode(m);
      }
  };

#endif // EDITTERRAINPANEL_H
