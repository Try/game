#ifndef GRAPHICSSETTINGSWIDGET_H
#define GRAPHICSSETTINGSWIDGET_H

#include "panel.h"

class GraphicsSettingsWidget:public Panel {
  public:
    GraphicsSettingsWidget( Resource & res );

    struct Settings{
      Settings();

      bool operator == ( const Settings& ) const;

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
      bool  transcurentShadows;

      int   physics;
      int   oreentation;

      enum API{
        openGL,
        directX
        };
      static API api;
      static void setSettings( const Settings & s );
      static const Settings& settings();

      static void save();
      static bool load();

      private:
        static Settings st;
        static const std::string &settingsFileName();

      friend class GraphicsSettingsWidget;
      };

    static Tempest::signal<const Settings&> onSettingsChanged;
  private:
    void shadowMapRes(int v);
    void smFilterQ(int v);
    void colorSh(bool v );
    void bloomQ(int v);
    void glow(bool v );
    void normalMap(bool v );
    void oclusion(bool v );
    void shTexture(bool v );

    void update();
  };

#endif // GRAPHICSSETTINGSWIDGET_H
