#include "guipass.h"

#include <MyGL/Render>
#include <MyGL/VertexBufferHolder>
#include <MyGL/Algo/PostProcessHelper>

#include "resource.h"
#include "gui/maingui.h"

GUIPass::GUIPass( const MyGL::VertexShader   & vsh,
                  const MyGL::FragmentShader & fsh,
                  MyGL::VertexBufferHolder &vbo,
                  MyGL::Size &s  )
  : vs(vsh), fs(fsh), vbHolder(vbo), size(s) {
  MyGL::VertexDeclaration::Declarator decl;
  decl.add( MyGL::Decl::float2, MyGL::Usage::Position )
      .add( MyGL::Decl::float2, MyGL::Usage::TexCoord );

  vdecl = MyGL::VertexDeclaration( vbo.device(), decl );

  guiRawData.reserve( 4*2048 );
  geometryBlocks.reserve( 512 );

  //noTexture = res.texture("gui/noTexture");
  //testTex   = res.texture("gui/frame");
  }

void GUIPass::exec( MainGui &gui, MyGL::Texture2d &rt,
                    MyGL::Texture2d &depth, MyGL::Device &device ) {
  dev = &device;

  if( gui.draw( *this ) )
    guiGeometry = vbHolder.load( guiRawData.data(), guiRawData.size() );

  MyGL::RenderState rs = makeRS( MyWidget::noBlend );

  device.setRenderState(rs);

  dTexCoord[0] = 1.0f/size.w;
  dTexCoord[1] = 1.0f/size.h;

  MyGL::RenderState currntRS = rs;
  device.beginPaint(rt, depth);
  device.setUniform( vs, dTexCoord, 2, "dTexCoord" );

  for( size_t i=0; i<geometryBlocks.size(); ++i ){
    const GeometryBlock& b = geometryBlocks[i];

    if( b.size && b.texture.tex ){
      device.setUniform( fs, b.texture.pageRawData(),  "texture" );

      if( currntRS!=b.state ){
        currntRS = b.state;
        device.setRenderState( b.state );
        }

      device.drawPrimitive( MyGL::AbstractAPI::Triangle,
                            vs, fs,
                            vdecl,
                            guiGeometry,
                            b.begin,
                            b.size/3 );
      }
    }

  device.endPaint();
  device.setRenderState( MyGL::RenderState() );
  }

void GUIPass::rect( int x0, int y0, int x1, int y1,
                    int texDx, int texDy, int tw, int th ) {
  //--tw;
  //--th;

  texDx += texRect.x;
  texDy += texRect.y;

  if( tw<0 || th<0 )
    return;

  Vertex v[4];
  v[0].x = x0; v[0].u = texDx;
  v[0].y = y0; v[0].v = texDy;

  v[1].x = x1; v[1].u = texDx + tw;
  v[1].y = y0; v[1].v = texDy;

  v[2].x = x1; v[2].u = texDx + tw;
  v[2].y = y1; v[2].v = texDy + th;

  v[3].x = x0; v[3].u = texDx;
  v[3].y = y1; v[3].v = texDy + th;

  for( int i=0; i<4; ++i ){
    v[i].x = -1 + 2.0*(v[i].x/size.w);
    v[i].y =  1 - 2.0*(v[i].y/size.h);

    v[i].u /= 2048.0;
    v[i].v /= 2048.0;
    }

  guiRawData.push_back( v[0] );
  guiRawData.push_back( v[1] );
  guiRawData.push_back( v[2] );

  guiRawData.push_back( v[0] );
  guiRawData.push_back( v[2] );
  guiRawData.push_back( v[3] );

  geometryBlocks.back().size += 6;
  }

void GUIPass::setTexture( const PixmapsPool::TexturePtr &t ) {
  texRect = t.rect;

  if( geometryBlocks.size() &&
      geometryBlocks.back().texture.tex==t.tex ){

    return;
    }

  GeometryBlock b;
  b.state = makeRS( MyWidget::noBlend );

  if( geometryBlocks.size() ){
    b = geometryBlocks.back();
    }

  b.texture = t;
  b.begin   = guiRawData.size();
  b.size    = 0;

  while( geometryBlocks.size() && geometryBlocks.back().size==0 )
    geometryBlocks.pop_back();

  geometryBlocks.push_back(b);
  }

void GUIPass::unsetTexture() {
  GeometryBlock b;
  b.texture.tex = 0;
  b.begin   = guiRawData.size();
  b.size    = 0;

  b.state = makeRS( MyWidget::noBlend );

  geometryBlocks.push_back(b);
  }

void GUIPass::clearBuffers() {
  guiRawData.clear();
  geometryBlocks.resize(1);
  geometryBlocks.back().begin   = 0;
  geometryBlocks.back().size    = 0;
  geometryBlocks.back().texture.tex = 0;

  geometryBlocks.back().state = makeRS( MyWidget::noBlend );
  }

void GUIPass::setBlendMode( MyWidget::BlendMode m ) {
  GeometryBlock b;
  b.texture.tex = 0;

  if( geometryBlocks.size() ){
    b = geometryBlocks.back();
    }

  b.begin   = guiRawData.size();
  b.state   = makeRS( m );
  b.size    = 0;

  geometryBlocks.push_back(b);
  }

MyGL::RenderState GUIPass::makeRS(MyWidget::BlendMode m) {
  MyGL::RenderState rs;
  rs.setZTest(false);
  rs.setZWriting( true );

  rs.setAlphaTestMode( MyGL::RenderState::AlphaTestMode::GEqual );
  rs.setBlend(0);
  rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::src_alpha,
                   MyGL::RenderState::AlphaBlendMode::one_minus_src_alpha );

  rs.setAlphaTestRef( 0.05 );

  if( m==MyWidget::addBlend ){
    rs.setBlend(1);
    rs.setZWriting(0);
    rs.setBlendMode( MyGL::RenderState::AlphaBlendMode::one,
                     MyGL::RenderState::AlphaBlendMode::one );
    }

  if( m==MyWidget::alphaBlend ){
    rs.setBlend(1);
    rs.setZWriting(0);
    }

  return rs;
  }
