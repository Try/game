#include "material.h"

#include <cstring>

float Material::wind = 0;

Material::Material():specular(0), useAlphaTest(1), alphaTrestRef(0.5) {
  memset( &usage, 0, sizeof(usage) );
  zWrighting = true;
  teamColor  = 0;
  }

MyGL::Matrix4x4 Material::animateObjMatrix( const MyGL::Matrix4x4 &object,
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

void Material::gbuffer(MyGL::RenderState &rs,
                        const MyGL::Matrix4x4 &object,
                        const MyGL::AbstractCamera &c,
                        MyGL::UniformTable & table ,
                        const MyGL::Matrix4x4 &shadowMatrix) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );

  if( usage.blush ){
    MyGL::Matrix4x4 mobj = animateObjMatrix(object);
    m.mul( mobj );
    } else {
    m.mul( object );
    }

  MyGL::Matrix4x4 sh = shadowMatrix;
  sh.mul( object );

  table.add( m,      "mvpMatrix",    MyGL::UniformTable::Vertex );
  table.add( object, "objectMatrix", MyGL::UniformTable::Vertex );
  table.add( sh,     "shadowMatrix", MyGL::UniformTable::Vertex );

  table.add( diffuse,  "texture",        MyGL::UniformTable::Fragment );
  table.add( normal,   "normalMap",      MyGL::UniformTable::Fragment );
  table.add( specular, "specularFactor", MyGL::UniformTable::Fragment );


  if( teamColor ){// FIXME
    float cl[3] = { teamColor->r(),
                    teamColor->g(),
                    teamColor->b() };
    table.add( cl, 3, "tmColor", MyGL::UniformTable::Fragment );
    } else {
    float cl[3] = {};
    table.add( cl, 3, "tmColor", MyGL::UniformTable::Fragment );
    }

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::grass( MyGL::RenderState &rs,
                      const MyGL::Matrix4x4 &object,
                      const MyGL::AbstractCamera &c,
                      MyGL::UniformTable &table,
                      const MyGL::Matrix4x4 &shadowMatrix) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );

  if( usage.blush ){
    MyGL::Matrix4x4 mobj = animateObjMatrix(object, 3);
    m.mul( mobj );
    } else {
    m.mul( object );
    }

  MyGL::Matrix4x4 sh = shadowMatrix;
  sh.mul( object );

  table.add( m,      "mvpMatrix",    MyGL::UniformTable::Vertex );
  table.add( object, "objectMatrix", MyGL::UniformTable::Vertex );
  table.add( sh,     "shadowMatrix", MyGL::UniformTable::Vertex );

  table.add( diffuse,  "texture",        MyGL::UniformTable::Fragment );
  table.add( specular, "specularFactor", MyGL::UniformTable::Fragment );

  rs.setZWriting(0);
  rs.setBlend(1);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                   MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::additive( MyGL::RenderState &rs,
                         const MyGL::Matrix4x4 &object,
                         const MyGL::AbstractCamera &c,
                         MyGL::UniformTable &u,
                         const MyGL::Matrix4x4 &) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix", MyGL::UniformTable::Vertex   );
  u.add( diffuse, "texture",  MyGL::UniformTable::Fragment );

  rs.setBlend(1);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::one,
                   MyGL::RenderState::AlphaBlendMode::one );

  rs.setZWriting(0);
  }

void Material::terrainMinor( MyGL::RenderState &rs,
                             const MyGL::Matrix4x4 &m,
                             const MyGL::AbstractCamera &c,
                             MyGL::UniformTable &u,
                             const MyGL::Matrix4x4 &sm ) const {
  gbuffer( rs, m, c, u, sm );

  rs.setZTestMode( MyGL::RenderState::ZTestMode::Equal );
  rs.setZWriting(0);

  rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::Always );
  rs.setAlphaTestRef(0.5);

  rs.setBlend(1);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::one,
                   MyGL::RenderState::AlphaBlendMode::one );
  }

void Material::terrainMinorZ( MyGL::RenderState &rs,
                              const MyGL::Matrix4x4 &m,
                              const MyGL::AbstractCamera &c,
                              MyGL::UniformTable &u,
                              const MyGL::Matrix4x4 &sm ) const {
  terrainMinor( rs, m, c, u, sm );

  rs.setZTestMode( MyGL::RenderState::ZTestMode::Less );
  rs.setZWriting(1);
  rs.setBlend(0);

  rs.setColorMask(0,0,0,0);
  }

void Material::transparent( MyGL::RenderState &rs,
                            const MyGL::Matrix4x4 &object,
                            const MyGL::AbstractCamera &c,
                            MyGL::UniformTable &table,
                            const MyGL::Matrix4x4 &shadowMatrix ) const {

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );

  if( usage.blush ){
    MyGL::Matrix4x4 mobj = animateObjMatrix(object);
    m.mul( mobj );
    } else {
    m.mul( object );
    }

  MyGL::Matrix4x4 sh = shadowMatrix;
  sh.mul( object );

  table.add( m,      "mvpMatrix",    MyGL::UniformTable::Vertex );
  table.add( object, "objectMatrix", MyGL::UniformTable::Vertex );
  table.add( sh,     "shadowMatrix", MyGL::UniformTable::Vertex );

  table.add( diffuse,  "texture",        MyGL::UniformTable::Fragment );
  table.add( normal,   "normalMap",      MyGL::UniformTable::Fragment );
  table.add( specular, "specularFactor", MyGL::UniformTable::Fragment );

  rs.setBlend(1);
  rs.setAlphaTestRef(0.01);
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                   MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );
  }

void Material::transparentZ(MyGL::RenderState &rs,
                            const MyGL::Matrix4x4 &m,
                            const MyGL::AbstractCamera &c,
                            MyGL::UniformTable &u,
                            const MyGL::Matrix4x4 &sm ) const {
  transparent( rs, m, c, u, sm );
  rs.setBlend(0);
  rs.setColorMask(0,0,0,0);
  }

void Material::glowPass( MyGL::RenderState & rs,
                         const MyGL::Matrix4x4 &object,
                         const MyGL::AbstractCamera &c,
                         MyGL::UniformTable & table ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  table.add( m,     "mvpMatrix",  MyGL::UniformTable::Vertex   );
  table.add( glow,  "texture",    MyGL::UniformTable::Fragment );
  }

void Material::shadow( MyGL::RenderState &rs,
                       const MyGL::Matrix4x4 &,
                       const MyGL::AbstractCamera &,
                       MyGL::UniformTable & table ) const {
  table.add( diffuse, "texture", MyGL::UniformTable::Fragment );

  rs.setCullFaceMode( MyGL::RenderState::CullMode::front );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::displace( MyGL::RenderState &rs,
                         const MyGL::Matrix4x4 &object,
                         const MyGL::AbstractCamera &c,
                         MyGL::UniformTable &table ,
                         const MyGL::Matrix4x4 &shadowMatrix ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);
  rs.setZWriting( true );

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  MyGL::Matrix4x4 vp = c.projective();
  vp.mul( c.view() );

  MyGL::Matrix4x4 sh = shadowMatrix;
  sh.mul( object );

  table.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex );
  table.add( vp,      "mvpMatrix",    MyGL::UniformTable::Fragment );

  table.add( object,  "objectMatrix", MyGL::UniformTable::Vertex );

  table.add( normal,  "normalMap",    MyGL::UniformTable::Fragment );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::water(MyGL::RenderState &rs,
                      const MyGL::Matrix4x4 &object,
                      const MyGL::AbstractCamera &c,
                      MyGL::UniformTable & table,
                      const MyGL::Matrix4x4 &shadowMatrix ) const {
  rs.setZTestMode( MyGL::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);
  rs.setZWriting( true );

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  MyGL::Matrix4x4 vp = c.projective();
  vp.mul( c.view() );

  MyGL::Matrix4x4 sh = shadowMatrix;
  sh.mul( object );

  table.add( vp,      "mvpMatrix",    MyGL::UniformTable::Fragment );
  table.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex   );
  table.add( object,  "objectMatrix", MyGL::UniformTable::Vertex   );
  table.add( sh,     "shadowMatrix",  MyGL::UniformTable::Vertex   );

  table.add( diffuse, "texture",      MyGL::UniformTable::Fragment );
  //table.add( normals, "normalMap",    MyGL::UniformTable::Fragment );
  //table.add( *shadowMap, "shadowMap", MyGL::UniformTable::Fragment );
  table.add( 1, "specularFactor", MyGL::UniformTable::Fragment );

  if( useAlphaTest ){
    rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::omni( MyGL::RenderState &rs,
                     const MyGL::Matrix4x4 &object,
                     const MyGL::AbstractCamera &c,
                     MyGL::UniformTable &u,
                     const MyGL::Matrix4x4 &sm,
                     const MyGL::Matrix4x4 &invMat ) const {
  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix",  MyGL::UniformTable::Vertex   );
  u.add( sm,     "shMatrix",   MyGL::UniformTable::Fragment );
  u.add( invMat, "invMatrix",  MyGL::UniformTable::Fragment );

  rs.setZWriting(0);
  rs.setBlend(1);

  rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::Greater );
  rs.setAlphaTestRef(0.0);

  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::one,
                   MyGL::RenderState::AlphaBlendMode::one );
  }

void Material::fogOfWar( MyGL::RenderState &rs,
                         const MyGL::Matrix4x4 &object,
                         const MyGL::AbstractCamera &c,
                         MyGL::UniformTable &table,
                         bool zpass ) const {

  if( !zpass ){
    rs.setZTestMode( MyGL::RenderState::ZTestMode::Equal );
    rs.setZTest( true );
    rs.setZWriting( false );
    }

  MyGL::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  table.add( m,       "mvpMatrix",    MyGL::UniformTable::Vertex   );
  table.add( object,  "objectMatrix", MyGL::UniformTable::Vertex   );

  rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
  rs.setAlphaTestRef ( 0.01 );

  if( !zpass ){
    rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                     MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );
    }
  }
