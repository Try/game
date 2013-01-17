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

  texSize = MyWidget::Size(2048);

  //noTexture = res.texture("gui/noTexture");
  //testTex   = res.texture("gui/frame");
  layers.reserve(8);
  setCurrentBuffer(0);
  }

void GUIPass::exec( MainGui &gui, MyGL::Texture2d &rt,
                    MyGL::Texture2d &depth, MyGL::Device &device ) {
  dev = &device;

  if( gui.draw( *this ) ){
    for( size_t i=0; i<layers.size(); ++i ){
      Layer& lay = layers[i];

      if( lay.needToUpdate ){
        lay.guiGeometry = MyGL::VertexBuffer<Vertex>();
        lay.guiGeometry = vbHolder.load( lay.guiRawData.data(),
                                         lay.guiRawData.size() );
        }
      }
    }

  MyGL::RenderState rs = makeRS( MyWidget::noBlend );

  device.setRenderState(rs);

  dTexCoord[0] = 1.0f/size.w;
  dTexCoord[1] = 1.0f/size.h;

  MyGL::RenderState currntRS = rs;
  device.beginPaint(rt, depth);
  device.setUniform( vs, dTexCoord, 2, "dTexCoord" );

  for( size_t r=0; r<layers.size(); ++r ){
    Layer& lay = layers[r];

    for( size_t i=0; i<lay.geometryBlocks.size(); ++i ){
      const GeometryBlock& b = lay.geometryBlocks[i];

      if( b.size && (b.texture.tex || b.texture.nonPool) ){
        if( b.texture.nonPool )
          device.setUniform( fs, *b.texture.nonPool,  "texture" ); else
          device.setUniform( fs, b.texture.pageRawData(),  "texture" );

        if( currntRS!=b.state ){
          currntRS = b.state;
          device.setRenderState( b.state );
          }

        device.drawPrimitive( MyGL::AbstractAPI::Triangle,
                              vs, fs,
                              vdecl,
                              lay.guiGeometry,
                              b.begin,
                              b.size/3 );
        }
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

    v[i].u /= texSize.w;
    v[i].v /= texSize.h;
    }

  Layer& lay = layers[curLay];
  lay.needToUpdate = true;

  lay.guiRawData.push_back( v[0] );
  lay.guiRawData.push_back( v[1] );
  lay.guiRawData.push_back( v[2] );

  lay.guiRawData.push_back( v[0] );
  lay.guiRawData.push_back( v[2] );
  lay.guiRawData.push_back( v[3] );

  lay.geometryBlocks.back().size += 6;
  }

void GUIPass::setTexture( const PixmapsPool::TexturePtr &t ) {
  Layer& lay = layers[curLay];

  texRect = t.rect;

  if( lay.geometryBlocks.size() &&
      lay.geometryBlocks.back().texture.tex     == t.tex &&
      lay.geometryBlocks.back().texture.id      == t.id  &&
      lay.geometryBlocks.back().texture.nonPool == t.nonPool ){

    return;
    }

  if( t.nonPool ){
    texRect = MyWidget::Rect(0,0, t.nonPool->width(), t.nonPool->height() );
    texSize = texRect.size();
    } else {
    if( t.tex ){
      MyGL::Texture2d &tx = (*t.tex)[ t.id ].t;
      texSize = MyWidget::Size(tx.width(), tx.height());
      }
    }

  GeometryBlock b;
  b.state = makeRS( MyWidget::noBlend );

  if( lay.geometryBlocks.size() ){
    b = lay.geometryBlocks.back();
    }

  b.texture = t;
  b.begin   = lay.guiRawData.size();
  b.size    = 0;

  while( lay.geometryBlocks.size() && lay.geometryBlocks.back().size==0 )
    lay.geometryBlocks.pop_back();

  lay.geometryBlocks.push_back(b);
  }

void GUIPass::unsetTexture() {
  Layer& lay = layers[curLay];

  GeometryBlock b;
  b.texture.tex = 0;
  b.begin   = lay.guiRawData.size();
  b.size    = 0;

  b.state = makeRS( MyWidget::noBlend );

  lay.geometryBlocks.push_back(b);
  }

void GUIPass::clearBuffers() {
  Layer& lay = layers[curLay];

  lay.needToUpdate = true;
  lay.guiRawData.clear();
  lay.geometryBlocks.resize(1);
  lay.geometryBlocks.back().begin   = 0;
  lay.geometryBlocks.back().size    = 0;
  lay.geometryBlocks.back().texture.tex = 0;

  lay.geometryBlocks.back().state = makeRS( MyWidget::noBlend );
  }

void GUIPass::setBlendMode( MyWidget::BlendMode m ) {
  Layer& lay = layers[curLay];

  GeometryBlock b;
  b.texture.tex = 0;

  if( lay.geometryBlocks.size() ){
    b = lay.geometryBlocks.back();
    }

  b.begin   = lay.guiRawData.size();
  b.state   = makeRS( m );
  b.size    = 0;

  lay.geometryBlocks.push_back(b);
  }

void GUIPass::setCurrentBuffer(int i) {
  curLay = i;

  while( layers.size() <= size_t(i) ){
    layers.push_back( Layer() );
    layers.back().geometryBlocks.reserve( 512 );
    layers.back().needToUpdate = false;
    layers.back().guiRawData.reserve( 4*2048 );
    }
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
