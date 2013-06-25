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
      bool  oclusion;
      bool  shadowTextures;

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
    void glow(bool v );
    void normalMap(bool v );
    void oclusion(bool v );
    void shTexture(bool v );
  };

#endif // GRAPHICSSETTINGSWIDGET_H
