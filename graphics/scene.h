#ifndef gSCENE_H
#define gSCENE_H

#include "material.h"
#include "octotree.h"

class Scene : public Tempest::AbstractScene< AbstractGraphicObject > {
  public:
    Scene( int sz );
    typedef OcTree<const AbstractGraphicObject*> Objects;

    const Objects& shadowCasters() const;
    const Objects& mainObjects() const;
    const Objects& glowObjects() const;
    const Objects& additiveObjects() const;

    const Objects& terrainObjects() const;
    const Objects& terrainMinorObjects() const;
    const Objects& displaceObjects() const;
    const Objects& waterObjects() const;
    const Objects& transparentObjects() const;
    const Objects& transparentZWObjects() const;

    const Objects& grassObjects() const;

    const Objects& fogOfWar() const;
    const Objects& omni() const;

  private:
    Objects shCast, mainObj, glowObj, terrainMinor, terrain,
            displaceObj, waterObj, addObj, transpObj,
            transpObjZW, fogOfWarObj,
            omniObj, grassObj;

    std::vector<Objects*> objPtrs;

    void onObjectAdded  ( const AbstractGraphicObject *);
    void onObjectRemoved( const AbstractGraphicObject *);
    void onObjectTransform(const AbstractGraphicObject *t, const Tempest::Matrix4x4&);

    enum Act{
      Add,
      Del,
      RePos
      };

    void onObjectOp( const AbstractGraphicObject *t, Act ins, const Tempest::Matrix4x4& );
    void onObjectOp( Objects & obj, const AbstractGraphicObject *t,
                     Act ins, const Tempest::Matrix4x4& );

    void remove( Objects & obj, const AbstractGraphicObject *t,
                 float x, float y, float z );
    void insert( Objects & obj, const AbstractGraphicObject *t,
                 float x, float y, float z );
  };

#endif // SCENE_H
