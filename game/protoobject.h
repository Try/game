#ifndef PROTOOBJECT_H
#define PROTOOBJECT_H

#include <string>
#include <vector>
#include <map>
#include <MyWidget/Shortcut>

struct ProtoObject {
  public:
    ProtoObject();

    bool isLandTile() const;

    struct GameSpecific{
      int size, speed;
      int visionRange;
      int maxHp;

      int gold, lim, limInc, buildTime;
      bool isBackground, invincible;

      struct Atack{
        int range;
        int damage, delay;

        std::string bullet;
        };

      std::vector<Atack> atk;
      std::map< std::string, std::string > propertyStr;

      const std::string &propStr( const std::string & s ) const;
      } data;

    struct View{
      View();

      std::string name;
      std::vector< std::string > materials;

      int   align[3];
      double size[3], alignSize;

      enum PhysicModel{
        NoModel,
        Sphere,
        Box
        } physModel;

      double sphereDiameter, specularFactor;
      double boxSize[3];

      bool randRotate;
      std::string isParticle;
      };

    struct SizeBounds{
      double min[3], max[3];
      } sizeBounds;

    std::vector<View>        view;
    std::vector<std::string> behaviors;
    std::vector<std::string> ability;
    double rotateSpeed;

    enum DeathAnim{
      NoAnim,
      Physic
      } deathAnim;

    std::string  name;
    std::wstring mouseHint;

    struct CmdButton{
      enum Action{
        NoAction,
        Buy,
        Build,
        Page,
        CastToGround,
        Count
        } action;

      std::string  taget, icon;
      std::wstring hint;
      MyWidget::KeyEvent::KeyType hotkey;
      int x,y;
      };
    struct Commans{
      struct Page{
        std::vector<CmdButton> btn;
        };

      std::vector<Page> pages;
      } commands;
  };

#endif // PROTOOBJECT_H
