#ifndef GRAPHICSSETTINGSWIDGET_H
#define GRAPHICSSETTINGSWIDGET_H

#include "panel.h"

class GraphicsSettingsWidget:public Panel {
  public:
    GraphicsSettingsWidget( Resource & res );

    struct Settings{
      Settings();

      enum Bloom{
        Off,
        Low,
        Hight
        };

      Bloom bloom;
      int   shadowMapRes;
      int   shadowFilterQ;
      bool  glow;
      bool  normalMap;

      enum API{
        openGL,
        directX
        };
      static API api;
      };

    Tempest::signal<const Settings&> onSettingsChanged;

  private:
    Settings s;

    void shadowMapRes(int v);
    void smFilterQ(int v);
    void bloomQ(int v);
    void glow( int v );
    void normalMap( int v );
  };

#endif // GRAPHICSSETTINGSWIDGET_H
