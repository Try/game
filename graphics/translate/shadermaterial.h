#ifndef SHADERMATERIAL_H
#define SHADERMATERIAL_H

#include <vector>
#include <string>

#include "shadersource.h"
#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

class ShaderMaterial {
  public:
    ShaderMaterial( Tempest::VertexShaderHolder&   vsHolder,
                    Tempest::FragmentShaderHolder& fsHolder );

    void install(ShaderSource &s , VertexInputAssembly &asemb);

    const ShaderSource::Code::Uniform& texUniform( size_t n ) const;
    size_t texUniformCount() const;

    void setTexture( int slot, const Tempest::Texture2d & t );

    struct UniformsContext{
      Tempest::Matrix4x4 mWorld, object, mvp;
      size_t texID;
      float view[3];

      int tick;

      float invW, invH;
      Tempest::Texture2d texture[ ShaderSource::tsCount ];
      };

    void setupShaderConst(UniformsContext& context );

    Tempest::VertexShader        vs;
    Tempest::FragmentShader      fs;
  private:
    struct Uniform{
      enum Usage{
        MVP,
        Obj,
        Texture,
        Time,
        dxScreenOffset,
        ViewVec,
        LightDir,
        LightColor,
        LightAblimient
        } usage;

      enum Shader{
        Vertex,
        Fragment
        } sh;

      ShaderSource::TextureSemantic tex;
      std::string name;
      };
    std::vector<Uniform> uniforms;

    struct Tex{
      ShaderSource::Code::Uniform u;
      Tempest::Texture2d          tex;
      };
    std::vector< Tex > utextures;

    ShaderSource::Code compiled;

    Tempest::VertexShaderHolder&   vsHolder;
    Tempest::FragmentShaderHolder& fsHolder;

    void mkUniform( const ShaderSource::Code::Uniform &u, Uniform& out );

    template< class T >
    void installUniform( T& sh,
                         const Uniform& u,
                         UniformsContext &c );
  };

#endif // SHADERMATERIAL_H
