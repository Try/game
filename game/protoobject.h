#ifndef PROTOOBJECT_H
#define PROTOOBJECT_H

#include <string>
#include <vector>
#include <map>
#include <Tempest/Shortcut>

struct ProtoObject {
  public:
    ProtoObject();

    bool isLandTile() const;

    struct GameSpecific{
      int size, speed;
      int visionRange;
      int maxHp;

      int gold, lim, limInc, buildTime;
      int acseleration;
      bool isBackground, invincible, isDynamic;
      std::vector<size_t> utype;

      int armor;

      struct Atack{
        Atack():range(0), damage(0), delay(0), uDestType(-1),
                splashSize(0), splashDamage(0), type(0){}

        int range;
        int damage, delay;
        size_t uDestType;

        int splashSize, splashDamage;

        std::string bullet;
        size_t type;
        };

      std::vector<Atack> atk;
      std::map< std::string, std::string > propertyStr;

      const std::string &propStr( const std::string & s ) const;
      } data;

    struct View{
      View();

      std::string name;
      std::vector< std::string > materials;
      size_t shadedMaterial;
      enum ShadowType{
        NoShadow,
        BasicShadow,
        BiasShadow
        };

      ShadowType shadowType;

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
      bool isLandDecal, hasOverDecal;
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
        Upgrade,
        Page,
        CastToGround,
        Count
        } action;

      std::string  taget, icon;
      std::wstring hint;
      Tempest::KeyEvent::KeyType hotkey;
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
