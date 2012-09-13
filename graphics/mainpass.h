#ifndef MAINPASS_H
#define MAINPASS_H

#include <MyGL/AbstractRenderPass>
#include <MyGL/VertexShader>
#include <MyGL/FragmentShader>

#include <MyGL/Algo/GBufferFillPass>

class MainPass : public MyGL::AbstractRenderPass {
  public:
    struct Buffer: public MyGL::GBufferFillPass::Buffer{
      Buffer( MyGL::TextureHolder & t, int w, int h ):
        MyGL::GBufferFillPass::Buffer(t,w,h){}

      using MyGL::GBufferFillPass::Buffer::data;
      };

    MainPass( Buffer & gbuffer,
              MyGL::Texture2d & depth,
              const MyGL::VertexShader   & vs,
              const MyGL::FragmentShader & fs );

    virtual void exec( const MyGL::Scene & s,
                       const MyGL::Scene::Objects &v,
                       MyGL::Device & d );

    MyGL::AbstractMaterial::MaterialID materialId() const;

    virtual MyGL::VertexShader&   vertexShader() { return vs; }
    virtual MyGL::FragmentShader& fragmentShader() { return fs; }

    virtual const MyGL::VertexShader&   vertexShader() const { return vs; }
    virtual const MyGL::FragmentShader& fragmentShader() const { return fs; }

  private:
    Buffer & buffer;
    MyGL::Texture2d & depth;

    MyGL::VertexShader   vs;
    MyGL::FragmentShader fs;
  };

#endif // MAINPASS_H
