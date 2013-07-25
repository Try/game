#ifndef GUIRENDER_H
#define GUIRENDER_H

#include "guipass.h"
#include "translate/shadermaterial.h"

class GuiRender {
  public:
    GuiRender( Tempest::VertexShaderHolder &vs,
               Tempest::FragmentShaderHolder &fs,
               Tempest::VertexBufferHolder &vbo,
               Tempest::IndexBufferHolder  &ibo,
               Tempest::Size &s,
               ShaderSource::Lang lang );

    void update( MainGui &gui,
                 Tempest::Device &device );
    void exec( MainGui &gui,
               Tempest::Texture2d *rt,
               Tempest::Texture2d *depth,
               Tempest::Device &device );
  private:
    struct Mat: ShaderMaterial{
      Mat( Tempest::VertexShaderHolder &vs,
           Tempest::FragmentShaderHolder &fs,
           ShaderSource::Lang lang );
      } matGui;
    GUIPass pass;
  };

#endif // GUIRENDER_H
