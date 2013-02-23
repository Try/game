#ifndef BLUSHMATERIAL_H
#define BLUSHMATERIAL_H

#include "mainmaterial.h"

class BlushMaterial : public MainMaterial {
  public:
    BlushMaterial( const MyGL::Matrix4x4 & shadowMatrix,
                   const MyGL::Color & teamColor );
    BlushMaterial( const MyGL::Matrix4x4 & shadowMatrix );

    static float wind;
    static MyGL::Matrix4x4 animateObjMatrix( const MyGL::Matrix4x4& object,
                                             double power = 1 );
  protected:
    bool bind( MyGL::RenderState &dev,
               const MyGL::Matrix4x4 &object,
               const MyGL::AbstractCamera &c,
               MyGL::UniformTable &) const;
  };

#endif // BLUSHMATERIAL_H
