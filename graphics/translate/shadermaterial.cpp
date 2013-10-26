#include "shadermaterial.h"
#include "mxassembly.h"
#include <Tempest/Device>

#include <iostream>

ShaderMaterial::ShaderMaterial( Tempest::VertexShaderHolder   &vs,
                                Tempest::FragmentShaderHolder &fs )
  :vsHolder(vs), fsHolder(fs) {
  utextures.reserve(8);
  }

void ShaderMaterial::install( ShaderSource &s,
                              const CompileOptions & opt,
                              VertexInputAssembly & asemb ) {
  uniforms.clear();
  utextures.clear();

  //MxAssembly asemb( MVertex::decl() );
  compiled = s.code( asemb, opt );

  vs = vsHolder.loadFromSource( compiled.vs );
  fs = fsHolder.loadFromSource( compiled.fs );

  if( !(vs.isValid() && fs.isValid()) ){
    std::cout << vs.log() << std::endl;
    std::cout << fs.log() << std::endl;
    }

  for( size_t i=0; i<compiled.uniformVs.size(); ++i ){
    const ShaderSource::Code::Uniform &u = compiled.uniformVs[i];
    Uniform ux;
    mkUniform(u, ux);
    ux.sh = Uniform::Vertex;
    uniforms.push_back(ux);
    }

  Tex tex;

  for( size_t i=0; i<compiled.uniformFs.size(); ++i ){
    const ShaderSource::Code::Uniform &u = compiled.uniformFs[i];
    if( u.type==ShaderSource::Code::texture ){
      tex.u = compiled.uniformFs[i];
      utextures.push_back(tex);
      }
    {
      Uniform ux;
      mkUniform(u, ux);
      ux.sh   = Uniform::Fragment;
      ux.tex  = u.texSemantic;
      ux.slot = u.slot;
      uniforms.push_back(ux);
      }
    }
  }

const ShaderSource::Code::Uniform &ShaderMaterial::texUniform(size_t n) const {
  return utextures[n].u;
  }

size_t ShaderMaterial::texUniformCount() const {
  return utextures.size();
  }

void ShaderMaterial::setTexture(int slot, const Tempest::Texture2d &t) {
  utextures[slot].tex = t;
  }

void ShaderMaterial::mkUniform( const ShaderSource::Code::Uniform &u,
                                ShaderMaterial::Uniform &ux ) {
  if( u.name=="mvpMatrix" ){
    ux.usage = Uniform::MVP;
    }
  if( u.name=="objMatrix" ){
    ux.usage = Uniform::Obj;
    }
  if( u.name=="worldMatrix" ){
    ux.usage = Uniform::WorldMat;
    }
  if( u.name=="shMatrix" ){
    ux.usage = Uniform::ShadowMat;
    }
  if( u.name=="time" ){
    ux.usage = Uniform::Time;
    }
  if( u.name=="lightDir" ){
    ux.usage = Uniform::LightDir;
    }
  if( u.name=="lightColor" ){
    ux.usage = Uniform::LightColor;
    }
  if( u.name=="dxScreenOffset" ){
    ux.usage = Uniform::dxScreenOffset;
    }
  if( u.name=="lightAblimient" ){
    ux.usage = Uniform::LightAblimient;
    }
  if( u.name=="viewVec" ){
    ux.usage = Uniform::ViewVec;
    }
  if( u.name=="color" ){
    ux.usage = Uniform::Color;
    }
  if( u.type==ShaderSource::Code::texture ){
    ux.usage = Uniform::Texture;
    }

  ux.tex  = ShaderSource::tsDiffuse;
  ux.name = u.name;
  }

void ShaderMaterial::setupShaderConst( UniformsContext& context ) {
  for( size_t i=0; i<uniforms.size(); ++i ){
    if( uniforms[i].sh==Uniform::Vertex )
      installUniform( vs, uniforms[i], context ); else
    if( uniforms[i].sh==Uniform::Fragment )
      installUniform( fs, uniforms[i], context );
    }
  }

const Tempest::RenderState &ShaderMaterial::renderState() const {
  return compiled.rs;
  }

void ShaderMaterial::setRenderState(const Tempest::RenderState &s) {
  compiled.rs = s;
  }

template< class T >
void ShaderMaterial::installUniform( T& vs,
                                     const Uniform& u ,
                                     UniformsContext &c ){
  float t = float( c.tick%1024 );

  switch ( u.usage ) {
    case Uniform::MVP:
      vs.setUniform( u.name.data(), c.mvp);
      break;
    case Uniform::Obj:
      vs.setUniform( u.name.data(), c.object );
      break;
    case Uniform::WorldMat:
      vs.setUniform( u.name.data(), c.mWorld);
      break;
    case Uniform::ShadowMat:
      vs.setUniform( u.name.data(), c.shMatrix);
      break;
    case Uniform::Time:
      vs.setUniform( u.name.data(), t );
      break;
    case Uniform::LightDir:{
      float d[3];
      d[0] = -c.lightDir[0];
      d[1] = -c.lightDir[1];
      d[2] = -c.lightDir[2];
      vs.setUniform( u.name.data(), d, 3 );
      }
      break;
    case Uniform::LightColor:
      vs.setUniform( u.name.data(), c.lightColor, 3 );
      break;
    case Uniform::LightAblimient:
      vs.setUniform( u.name.data(), c.sceneAblimient, 3 );
      break;
    case Uniform::Color:
      vs.setUniform( u.name.data(), c.color.data(), 4 );
      break;
    case Uniform::dxScreenOffset:{
      float w[2] = { -c.invW, c.invH };
      vs.setUniform( u.name.data(), w, 2 );
      }
      break;
    case Uniform::Texture:{
      if( !c.texture[u.tex][u.slot].isEmpty() )
        vs.setUniform( u.name.data(), c.texture[u.tex][u.slot] );else
      if( c.texID < utextures.size() )
        vs.setUniform( u.name.data(), utextures[c.texID].tex );
      ++c.texID;
      }
      break;

    case Uniform::ViewVec :
      float v[3] = { -c.view[0],
                     -c.view[1],
                     -c.view[2] };
      vs.setUniform( u.name.data(), v, 3 );
      break;
    }
  }
