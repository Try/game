#include "transparentmaterial.h"

#include <MyGL/UniformTable>
#include <MyGL/RenderState>
#include <MyGL/AbstractCamera>

TransparentMaterial::TransparentMaterial(const MyGL::Matrix4x4 &sm)
                    :shadowMatrix(&sm) {
  specular = 1;
  }

bool TransparentMaterial::bind( MyGL::RenderState &rs,
                                const MyGL::Matrix4x4 &object,
                                const MyGL::AbstractCamera &c,
                                MyGL::UniformTable & u) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  MyGL::Matrix4x4 sh = *shadowMatrix;
  sh.mul( object );

  u.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex );
  u.add( object,  "objectMatrix", MyGL::UniformTable::Vertex );
  u.add( sh,      "shadowMatrix", MyGL::UniformTable::Vertex );

  u.add( texture,   "texture",       MyGL::UniformTable::Fragment );
  u.add( normalMap, "normalMap",     MyGL::UniformTable::Fragment );
  u.add( specular, "specularFactor", MyGL::UniformTable::Fragment );

  rs.setBlend(1);
  rs.setAlphaTestRef(0.01);
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                   MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );

  return 1;
  }

bool TransparentMaterialZPass::bind( MyGL::RenderState &rs,
                                const MyGL::Matrix4x4 &object,
                                const MyGL::AbstractCamera &c,
                                MyGL::UniformTable & u) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix", MyGL::UniformTable::Vertex   );
  u.add( texture, "texture",  MyGL::UniformTable::Fragment );

  rs.setAlphaTestRef(0.01);
  rs.setColorMask(0,0,0,0);

  return 1;
  }


TransparentMaterialNoZW::TransparentMaterialNoZW(const MyGL::Matrix4x4 &s)
                        :TransparentMaterial(s){

  }

bool TransparentMaterialNoZW::bind( MyGL::RenderState &rs,
                                    const MyGL::Matrix4x4 &object,
                                    const MyGL::AbstractCamera &c,
                                    MyGL::UniformTable & u ) const {
  TransparentMaterial::bind( rs, object, c, u );
  rs.setZWriting(0);

  return 1;
  }


TransparentMaterialShadow::TransparentMaterialShadow(const MyGL::Matrix4x4 &s)
                          :TransparentMaterial(s){

  }

bool TransparentMaterialShadow::bind( MyGL::RenderState &rs,
                                    const MyGL::Matrix4x4 &object,
                                    const MyGL::AbstractCamera &c,
                                    MyGL::UniformTable & u ) const {
  TransparentMaterial::bind( rs, object, c, u );
  rs.setZTest(0);

  //rs.setBlend(0);
  return 1;
  }
