#include "guirender.h"

#include "translate/guiassembly.h"
#include "translate/objectcode.h"

GuiRender::GuiRender(Tempest::VertexShaderHolder &vs,
                      Tempest::FragmentShaderHolder &fs,
                      Tempest::VertexBufferHolder &vbo,
                      Tempest::IndexBufferHolder &ibo,
                      Tempest::Size &s , ShaderSource::Lang lang)
  :matGui(vs,fs, lang), pass(matGui.vs, matGui.fs, vbo, ibo, s){
  for( size_t i=0; i<matGui.texUniformCount(); ++i )
    pass.utextureName = matGui.texUniform(i).name;
  }

void GuiRender::update(MainGui &gui, Tempest::Device &device) {
  return pass.update(gui, device);
  }

void GuiRender::exec( MainGui &gui, Tempest::Texture2d *rt,
                      Tempest::Texture2d *depth, Tempest::Device &device) {
  pass.exec(gui,rt,depth,device);
  }

GuiRender::Mat::Mat( Tempest::VertexShaderHolder &vs,
                     Tempest::FragmentShaderHolder &fs,
                     ShaderSource::Lang lang )
  :ShaderMaterial(vs,fs){
  ObjectCode co;
  co.loadFromFile( L"data/sh/material/gui.json" );

  GuiAssembly assemb( GUIPass::decl(), lang );
  install( *co.codeOf(), assemb );
  }
