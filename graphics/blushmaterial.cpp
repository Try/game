#include "blushmaterial.h"

#include <MyGL/UniformTable>

float BlushMaterial::wind = 0.0;

BlushMaterial::BlushMaterial(const MyGL::Matrix4x4 &shadowMatrix,
                             const MyGL::Color &teamColor)
  :MainMaterial(shadowMatrix, teamColor) {
  }

BlushMaterial::BlushMaterial(const MyGL::Matrix4x4 &shadowMatrix)
  :MainMaterial(shadowMatrix) {
  }

MyGL::Matrix4x4 BlushMaterial::animateObjMatrix( const MyGL::Matrix4x4 &object,
                                                 double power ) {
  MyGL::Float dvec[3] = { power*0.02*wind, power*0.03*wind, 0};

  MyGL::Float mx[4][4] = {
    {1,0,0,0},
    {0,1,0,0},
    { dvec[0], dvec[1], 1,0},
    {0,0,0,1}
    };

  MyGL::Matrix4x4 mobj( (MyGL::Float*)mx );
  //mobj.mul( object );
  MyGL::Matrix4x4 objR = object;
  objR.set(3,0, 0);
  objR.set(3,1, 0);
  objR.set(3,2, 0);

  mobj.mul( objR );

  mobj.set(3,0, object.at(3,0));
  mobj.set(3,1, object.at(3,1));
  mobj.set(3,2, object.at(3,2));

  return mobj;
  }

bool BlushMaterial::bind( MyGL::RenderState &dev,
                          const MyGL::Matrix4x4 &object,
                          const MyGL::AbstractCamera &c,
                          MyGL::UniformTable & u) const {
  MainMaterial::bind( dev, object, c, u );

  MyGL::Matrix4x4 mobj = animateObjMatrix(object);
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( mobj );

  MyGL::Matrix4x4 sh = *shadowMatrix;
  sh.mul( mobj );

  u.add( m,      "mvpMatrix",    MyGL::UniformTable::Vertex );
  u.add( mobj,   "objectMatrix", MyGL::UniformTable::Vertex );
  u.add( sh,     "shadowMatrix", MyGL::UniformTable::Vertex );

  return 1;
  }


BlushShMaterial::BlushShMaterial() {

  }
