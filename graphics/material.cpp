#include "material.h"

#include <cstring>

float Material::wind = 0;

GraphicsSettingsWidget::Settings Material::settings;

Material::Material():specular(0), useAlphaTest(1), alphaTrestRef(0.5) {
  memset( &usage, 0, sizeof(usage) );
  zWrighting = true;
  teamColor  = 0;
  }

Tempest::Matrix4x4 Material::animateObjMatrix( const Tempest::Matrix4x4 &object,
                                               float power ) {
  float dvec[3] = { power*0.02f*wind, power*0.03f*wind, 0};

  float mx[4][4] = {
    {1,0,0,0},
    {0,1,0,0},
    { dvec[0], dvec[1], 1,0},
    {0,0,0,1}
    };

  Tempest::Matrix4x4 mobj( (float*)mx );
  //mobj.mul( object );
  Tempest::Matrix4x4 objR = object;
  objR.set(3,0, 0);
  objR.set(3,1, 0);
  objR.set(3,2, 0);

  mobj.mul( objR );

  mobj.set(3,0, object.at(3,0));
  mobj.set(3,1, object.at(3,1));
  mobj.set(3,2, object.at(3,2));

  return mobj;
  }

void Material::gbuffer( Tempest::RenderState &rs,
                        const Tempest::Matrix4x4 &object,
                        const Tempest::AbstractCamera &c,
                        Tempest::UniformTable & table ) const {
  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );

  if( usage.blush ){
    Tempest::Matrix4x4 mobj = animateObjMatrix(object);
    m.mul( mobj );
    } else {
    m.mul( object );
    }

  //Tempest::Matrix4x4 sh = shadowMatrix;
  //sh.mul( object );

  table.add( m,      "mvpMatrix",    Tempest::UniformTable::Vertex );
  table.add( object, "objectMatrix", Tempest::UniformTable::Vertex );
  //table.add( sh,     "shadowMatrix", Tempest::UniformTable::Vertex );

  table.add( diffuse,  "texture",        Tempest::UniformTable::Fragment );
  if( settings.normalMap )
    table.add( normal,   "normalMap",      Tempest::UniformTable::Fragment );
  table.add( specular, "specularFactor", Tempest::UniformTable::Fragment );

  if( teamColor ){// FIXME
    float cl[3] = { teamColor->r(),
                    teamColor->g(),
                    teamColor->b() };
    table.add( cl, 3, "tmColor", Tempest::UniformTable::Fragment );
    } else {
    float cl[3] = {};
    table.add( cl, 3, "tmColor", Tempest::UniformTable::Fragment );
    }

  if( useAlphaTest ){
    rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::grass( Tempest::RenderState &rs,
                      const Tempest::Matrix4x4 &object,
                      const Tempest::AbstractCamera &c,
                      Tempest::UniformTable &table ) const {
  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );

  if( usage.blush ){
    Tempest::Matrix4x4 mobj = animateObjMatrix(object, 3);
    m.mul( mobj );
    } else {
    m.mul( object );
    }

  //Tempest::Matrix4x4 sh = shadowMatrix;
  //sh.mul( object );

  table.add( m,      "mvpMatrix",    Tempest::UniformTable::Vertex );
  table.add( object, "objectMatrix", Tempest::UniformTable::Vertex );
  //table.add( sh,     "shadowMatrix", Tempest::UniformTable::Vertex );

  table.add( diffuse,  "texture",        Tempest::UniformTable::Fragment );
  table.add( specular, "specularFactor", Tempest::UniformTable::Fragment );

  rs.setZWriting(0);
  rs.setBlend(1);
  rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::src_alpha,
                   Tempest::RenderState::AlphaBlendMode::one_minus_src_alpha );

  if( useAlphaTest ){
    rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::additive( Tempest::RenderState &rs,
                         const Tempest::Matrix4x4 &object,
                         const Tempest::AbstractCamera &c,
                         Tempest::UniformTable &u ) const {
  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix", Tempest::UniformTable::Vertex   );
  u.add( diffuse, "texture",  Tempest::UniformTable::Fragment );

  rs.setZTestMode( Tempest::RenderState::ZTestMode::LEqual );
  rs.setBlend(1);
  rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::one,
                   Tempest::RenderState::AlphaBlendMode::one );

  rs.setZWriting(0);
  }

void Material::terrainMain( Tempest::RenderState & rs,
                            const Tempest::Matrix4x4 &m,
                            const Tempest::AbstractCamera &c,
                            Tempest::UniformTable &u ) const {
  gbuffer( rs, m, c, u );
  //return;
  //rs.setPolygonRenderMode( Tempest::RenderState::PolyRenderMode::Line );
  //return;
  if(0){
    rs.setZWriting(0);
    rs.setBlend(1);
    rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::one,
                     Tempest::RenderState::AlphaBlendMode::one );
    }
  }

void Material::terrainMinor( Tempest::RenderState &rs,
                             const Tempest::Matrix4x4 &m,
                             const Tempest::AbstractCamera &c,
                             Tempest::UniformTable &u ) const {
  gbuffer( rs, m, c, u );

  rs.setZTestMode( Tempest::RenderState::ZTestMode::Equal );
  rs.setZWriting(0);

  rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::Always );
  rs.setAlphaTestRef(0.5);

  rs.setBlend(1);
  rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::one,
                   Tempest::RenderState::AlphaBlendMode::one );
  }

void Material::terrainMinorZ( Tempest::RenderState &rs,
                              const Tempest::Matrix4x4 &m,
                              const Tempest::AbstractCamera &c,
                              Tempest::UniformTable &u ) const {
  terrainMinor( rs, m, c, u );

  rs.setZTestMode( Tempest::RenderState::ZTestMode::Less );
  rs.setZWriting(1);
  rs.setBlend(0);

  rs.setColorMask(0,0,0,0);
  }

void Material::transparent(Tempest::RenderState &rs,
                            const Tempest::Matrix4x4 &object,
                            const Tempest::AbstractCamera &c,
                            Tempest::UniformTable &table) const {

  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );

  if( usage.blush ){
    Tempest::Matrix4x4 mobj = animateObjMatrix(object);
    m.mul( mobj );
    } else {
    m.mul( object );
    }

  //Tempest::Matrix4x4 sh = shadowMatrix;
  //sh.mul( object );

  table.add( m,      "mvpMatrix",    Tempest::UniformTable::Vertex );
  table.add( object, "objectMatrix", Tempest::UniformTable::Vertex );
  //table.add( sh,     "shadowMatrix", Tempest::UniformTable::Vertex );

  table.add( diffuse,  "texture",        Tempest::UniformTable::Fragment );
  table.add( normal,   "normalMap",      Tempest::UniformTable::Fragment );
  table.add( specular, "specularFactor", Tempest::UniformTable::Fragment );

  rs.setBlend(1);
  rs.setAlphaTestRef(0.01);
  rs.setZTestMode( Tempest::RenderState::ZTestMode::LEqual );
  rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::src_alpha,
                   Tempest::RenderState::AlphaBlendMode::one_minus_src_alpha );
  rs.setZWriting( zWrighting );
  }

void Material::transparentZ(Tempest::RenderState &rs,
                            const Tempest::Matrix4x4 &m,
                            const Tempest::AbstractCamera &c,
                            Tempest::UniformTable &u ) const {
  transparent( rs, m, c, u );
  rs.setBlend(0);
  rs.setColorMask(0,0,0,0);
  }

void Material::glowPass(Tempest::RenderState & rs,
                         const Tempest::Matrix4x4 &object,
                         const Tempest::AbstractCamera &c,
                         Tempest::UniformTable & table ) const {
  rs.setZTestMode( Tempest::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);
  rs.setZWriting(false);

  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  table.add( m,     "mvpMatrix",  Tempest::UniformTable::Vertex   );
  table.add( glow,  "texture",    Tempest::UniformTable::Fragment );
  }

void Material::glowPassAdd( Tempest::RenderState &rs,
                            const Tempest::Matrix4x4 &obj,
                            const Tempest::AbstractCamera &c,
                            Tempest::UniformTable &t) const {
  glowPass(rs, obj, c, t);
  rs.setBlend(1);
  rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::one,
                   Tempest::RenderState::AlphaBlendMode::one );
  }

void Material::shadow( Tempest::RenderState &rs,
                       const Tempest::Matrix4x4 &object,
                       const Tempest::AbstractCamera &,
                       Tempest::UniformTable & table,
                       const Tempest::Matrix4x4 &sh ) const {
  Tempest::Matrix4x4 m = sh;

  if( !usage.blush )
    m.mul( object ); else
    m.mul( Material::animateObjMatrix( object ) );

  table.add( m,     "mvpMatrix", Tempest::UniformTable::Vertex );
  table.add( diffuse, "texture", Tempest::UniformTable::Fragment );

  rs.setCullFaceMode( Tempest::RenderState::CullMode::front );

  if( useAlphaTest ){
    rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::displace(Tempest::RenderState &rs,
                         const Tempest::Matrix4x4 &object,
                         const Tempest::AbstractCamera &c,
                         Tempest::UniformTable &table ) const {
  rs.setZTestMode( Tempest::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);
  rs.setZWriting( true );

  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  Tempest::Matrix4x4 vp = c.projective();
  vp.mul( c.view() );

  //Tempest::Matrix4x4 sh = shadowMatrix;
  //sh.mul( object );

  table.add( m,       "mvpMatrix",    Tempest::UniformTable::Vertex );
  table.add( vp,      "mvpMatrix",    Tempest::UniformTable::Fragment );

  table.add( object,  "objectMatrix", Tempest::UniformTable::Vertex );

  table.add( normal,  "normalMap",    Tempest::UniformTable::Fragment );

  if( useAlphaTest ){
    rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::water(Tempest::RenderState &rs,
                      const Tempest::Matrix4x4 &object,
                      const Tempest::AbstractCamera &c,
                      Tempest::UniformTable & table) const {
  rs.setZTestMode( Tempest::RenderState::ZTestMode::LEqual );
  rs.setZTest(true);
  rs.setZWriting( true );

  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  Tempest::Matrix4x4 vp = c.projective();
  vp.mul( c.view() );

  //Tempest::Matrix4x4 sh = shadowMatrix;
  //sh.mul( object );

  Tempest::Matrix4x4 invMat = c.projective();
  invMat.mul( c.view() );
  invMat.mul( object );
  invMat.inverse();

  table.add( vp,      "mvpMatrix",    Tempest::UniformTable::Fragment );
  table.add( m,       "mvpMatrix",    Tempest::UniformTable::Vertex   );
  table.add( object,  "objectMatrix", Tempest::UniformTable::Vertex   );
  table.add( invMat,  "invMatrix",    Tempest::UniformTable::Fragment );

  table.add( diffuse, "texture",      Tempest::UniformTable::Fragment );
  table.add( 1, "specularFactor",     Tempest::UniformTable::Fragment );

  if( useAlphaTest ){
    rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::GEqual );
    rs.setAlphaTestRef( alphaTrestRef );
    }
  }

void Material::omni( Tempest::RenderState &rs,
                     const Tempest::Matrix4x4 &object,
                     const Tempest::AbstractCamera &c,
                     Tempest::UniformTable &u,
                     const Tempest::Matrix4x4 &shM ) const {
  Tempest::Matrix4x4 invMat = c.projective();
  invMat.mul( c.view() );
  invMat.mul( object );
  invMat.inverse();

  Tempest::Matrix4x4 sm = shM;
  sm.mul( object );

  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  u.add( m,      "mvpMatrix",  Tempest::UniformTable::Vertex   );
  u.add( sm,     "shMatrix",   Tempest::UniformTable::Fragment );
  u.add( invMat, "invMatrix",  Tempest::UniformTable::Fragment );

  rs.setZWriting(0);
  rs.setBlend(1);

  rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::Greater );
  rs.setAlphaTestRef(0.0);

  rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::one,
                   Tempest::RenderState::AlphaBlendMode::one );
  }

void Material::fogOfWar( Tempest::RenderState &rs,
                         const Tempest::Matrix4x4 &object,
                         const Tempest::AbstractCamera &c,
                         Tempest::UniformTable &table,
                         bool zpass ) const {

  if( !zpass ){
    rs.setZTestMode( Tempest::RenderState::ZTestMode::Equal );
    rs.setZTest( true );
    rs.setZWriting( false );
    }

  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( object );

  table.add( m,       "mvpMatrix",    Tempest::UniformTable::Vertex   );
  table.add( object,  "objectMatrix", Tempest::UniformTable::Vertex   );

  rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::Always );
  }
