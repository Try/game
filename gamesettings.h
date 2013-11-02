#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <Tempest/Color>
#include <string>

class GameSettings {
  public:
    GameSettings();

    static int difficulty;
    static Tempest::Color color;
    static bool smallMenu;

    static void save( const char* file );
    static void load( const char* file );

    static void save();
    static void load();

    static const std::string& fileName();
  };

#endif // GAMESETTINGS_H
