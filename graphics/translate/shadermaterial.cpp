#include "shadermaterial.h"
#include "mxassembly.h"

#include <iostream>

ShaderMaterial::ShaderMaterial( Tempest::VertexShaderHolder   &vs,
                                Tempest::FragmentShaderHolder &fs )
  :vsHolder(vs), fsHolder(fs) {
  utextures.reserve(8);
  }

void ShaderMaterial::install( ShaderSource &s,
                              VertexInputAssembly & asemb ) {
  uniforms.clear();

  //MxAssembly asemb( MVertex::decl() );
  compiled = s.code( asemb, asemb.getLang() );

  std::cout << compiled.vs << std::endl;
  std::cout << compiled.fs << std::endl;

  vs = vsHolder.loadFromSource( compiled.vs );
  fs = fsHolder.loadFromSource( compiled.fs );

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
      ux.sh = Uniform::Fragment;
      ux.tex = u.texSemantic;
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
  if( u.type==ShaderSource::Code::texture ){
    ux.usage = Uniform::Texture;
    }

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

template< class T >
void ShaderMaterial::installUniform( T& vs,
                                     const Uniform& u ,
                                     UniformsContext &c ){
  float t = float( c.tick%1024 );
  float ldir[3] = {0.301511, 0.301511, 0.904534};

  float lcl[3] = {0.7f, 0.7f, 0.7f};
  float lab[3] = {0.3f, 0.3f, 0.3f};

  switch ( u.usage ) {
    case Uniform::MVP:
      vs.setUniform( u.name.data(), c.mvp);
      break;
    case Uniform::Obj:
      vs.setUniform( u.name.data(), c.object );
      break;
    case Uniform::Time:
      vs.setUniform( u.name.data(), t );
      break;
    case Uniform::LightDir:
      vs.setUniform( u.name.data(), ldir, 3 );
      break;
    case Uniform::LightColor:
      vs.setUniform( u.name.data(), lcl, 3 );
      break;
    case Uniform::LightAblimient:
      vs.setUniform( u.name.data(), lab, 3 );
      break;
    case Uniform::dxScreenOffset:{
      float w[2] = { -c.invW, c.invH };
      vs.setUniform( u.name.data(), w, 2 );
      }
      break;
    case Uniform::Texture:{
      if( !c.texture[u.tex].isEmpty() )
        vs.setUniform( u.name.data(), c.texture[u.tex] );else
      if( c.texID < utextures.size() )
        vs.setUniform( u.name.data(), utextures[c.texID].tex );
      ++c.texID;
      }
      break;

    case Uniform::ViewVec :
      vs.setUniform( u.name.data(), c.view, 3 );
      break;
    }
  }
