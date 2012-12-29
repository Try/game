#ifndef TERRAINMINORMATERIAL_H
#define TERRAINMINORMATERIAL_H

#include "mainmaterial.h"

class TerrainMinorMaterial : public MainMaterial {
  public:
    TerrainMinorMaterial( const MyGL::Matrix4x4 & shadowMatrix );

  protected:
    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;
  };

class TerrainZPass : public MyGL::AbstractMaterial {
  public:
    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;
    MyGL::Texture2d texture;
};

#endif // TERRAINMINORMATERIAL_H
