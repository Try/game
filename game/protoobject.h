#ifndef PROTOOBJECT_H
#define PROTOOBJECT_H

#include <string>
#include <vector>
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
      bool isBackground;

      struct Atack{
        int range;
        int damage, delay;

        std::string bullet;
        };

      std::vector<Atack> atk;
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

      bool randRotate, isParticle;
      };

    struct SizeBounds{
      double min[3], max[3];
      } sizeBounds;

    std::vector<View>        view;
    std::vector<std::string> behaviors;
    double rotateSpeed;

    std::string name;

    struct CmdButton{
      enum Action{
        NoAction,
        Buy,
        Build,
        Page,
        Count
        } action;

      std::string taget, icon;
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
