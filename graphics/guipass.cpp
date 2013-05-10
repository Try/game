#include "guipass.h"

#include <Tempest/Render>
#include <Tempest/VertexBufferHolder>
#include <Tempest/PostProcessHelper>

#include "resource.h"
#include "gui/maingui.h"

#include <iostream>

GUIPass::GUIPass( const Tempest::VertexShader   & vsh,
                  const Tempest::FragmentShader & fsh,
                  Tempest::VertexBufferHolder &vbo,
                  Tempest::IndexBufferHolder  &ibo,
                  Tempest::Size &s  )
  : vs(vsh), fs(fsh), vbHolder(vbo), ibHolder(ibo), size(s) {
  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::half2, Tempest::Usage::Position )
      .add( Tempest::Decl::half2, Tempest::Usage::TexCoord, 0 )
      .add( Tempest::Decl::half4, Tempest::Usage::TexCoord, 1 );

  vdecl = Tempest::VertexDeclaration( vbo.device(), decl );

  texSize = Tempest::Size(2048);

  //noTexture = res.texture("gui/noTexture");
  //testTex   = res.texture("gui/frame");
  layers.reserve(8);
  setCurrentBuffer(0);

  setColor(1,1,1,1);
  iboTmp.reserve( 8096 );
  }

void GUIPass::exec( MainGui &gui,
                    Tempest::Texture2d *rt,
                    Tempest::Texture2d *depth,
                    Tempest::Device &device ) {
  setColor(1,1,1,1);
  dev = &device;

  if( gui.draw( *this ) ){
    size_t sz = 0;
    for( size_t i=0; i<layers.size(); ++i ){
      Layer& lay = layers[i];
      sz = std::max( lay.guiRawData.size(), sz );

      if( lay.needToUpdate ){
        lay.guiGeometry = Tempest::VertexBuffer<HVertex>();
        lay.guiGeometry = vbHolder.load( lay.guiRawData );
        lay.needToUpdate = false;
        }
      }

    if( sz && int(sz)>=guiIndex.size() ){
      guiIndex    = Tempest::IndexBuffer<uint16_t>();
      iboTmp.resize( 6*sz/4 );

      for( size_t i=0, id = 0; i<iboTmp.size(); i+=6, id+=4 ){
        uint16_t *ib = &iboTmp[i];
        ib[0] = id+0;
        ib[1] = id+1;
        ib[2] = id+2;

        ib[3] = id+0;
        ib[4] = id+2;
        ib[5] = id+3;
        }

      guiIndex = ibHolder.load( iboTmp );
      }
    }

  //std::cerr <<"guiIndex = " << guiIndex.size() << std::endl;

  Tempest::RenderState rs = makeRS( Tempest::noBlend );

  device.setRenderState(rs);

  dTexCoord[0] = 1.0f/size.w;
  dTexCoord[1] = 1.0f/size.h;

  Tempest::RenderState currntRS = rs;

  if( rt )
    device.beginPaint(*rt, *depth); else
    device.beginPaint();

  //device.clear( Tempest::Color(0,0,0,1) );//FOR DROD TESTS

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

        device.drawIndexed( Tempest::AbstractAPI::Triangle,
                            vs, fs,
                            vdecl,
                            lay.guiGeometry,
                            guiIndex,
                            b.begin,
                            0,
                            b.size/2 );
        }
      }
    }

  device.endPaint();
  device.setRenderState( Tempest::RenderState() );

  stateStk.clear();
  }

void GUIPass::rect( int x0, int y0, int x1, int y1,
                    int texDx, int texDy, int tw, int th ) {
  //--tw;
  //--th;

  texDx += texRect.x;
  texDy += texRect.y;

  if( tw<0 || th<0 )
    return;

  HVertex v[4];
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

    v[i].u = v[i].u/texSize.w;
    v[i].v = v[i].v/texSize.h;

    std::copy( state.color, state.color+4, v[i].color );
    }

  Layer& lay = layers[curLay];
  lay.needToUpdate = true;

  lay.guiRawData.push_back( v[0] );
  lay.guiRawData.push_back( v[1] );
  lay.guiRawData.push_back( v[2] );

  //lay.guiRawData.push_back( v[0] );
  //lay.guiRawData.push_back( v[2] );
  lay.guiRawData.push_back( v[3] );

  lay.geometryBlocks.back().size += 4;
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

  if( t.tex==0 && t.nonPool==0 )
    return;

  if( t.nonPool ){
    texRect = Tempest::Rect(0,0, t.nonPool->width(), t.nonPool->height() );
    texSize = texRect.size();
    } else {
    if( t.tex ){
      Tempest::Texture2d &tx = (*t.tex)[ t.id ].t;
      texSize = Tempest::Size(tx.width(), tx.height());
      }
    }

  GeometryBlock b;
  b.state = makeRS( Tempest::noBlend );

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
  return;

  Layer& lay = layers[curLay];

  GeometryBlock b;
  b.texture.tex = 0;
  b.begin   = lay.guiRawData.size();
  b.size    = 0;

  b.state = makeRS( Tempest::noBlend );

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

  lay.geometryBlocks.back().state = makeRS( Tempest::noBlend );
  }

void GUIPass::setBlendMode( Tempest::BlendMode m ) {
  Layer& lay = layers[curLay];

  GeometryBlock b;
  b.texture.tex = 0;

  if( lay.geometryBlocks.size() ){
    b = lay.geometryBlocks.back();
    }

  bool e = false;
  while( lay.geometryBlocks.size() && lay.geometryBlocks.back().size==0 ){
    lay.geometryBlocks.pop_back();
    e = true;
    }

  Tempest::RenderState rs = makeRS( m );
  if( b.state!=rs || e ){
    b.begin   = lay.guiRawData.size();
    b.state   = rs;
    b.size    = 0;

    lay.geometryBlocks.push_back(b);
    }
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

void GUIPass::setColor(float r, float g, float b, float a) {
  state.color[0] = r;
  state.color[1] = g;
  state.color[2] = b;
  state.color[3] = a;
  }

void GUIPass::pushState() {
  stateStk.push_back( state );
  }

void GUIPass::popState() {
  state = stateStk.back();
  stateStk.pop_back();

  setColor( state.color[0],
            state.color[1],
            state.color[2],
            state.color[3] );
  }

Tempest::RenderState GUIPass::makeRS(Tempest::BlendMode m) {
  Tempest::RenderState rs;
  rs.setZTest(false);
  rs.setZWriting( true );
  //rs.setPolygonRenderMode( Tempest::RenderState::PolyRenderMode::Line );

  rs.setAlphaTestMode( Tempest::RenderState::AlphaTestMode::GEqual );
  rs.setBlend(0);
  rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::src_alpha,
                   Tempest::RenderState::AlphaBlendMode::one_minus_src_alpha );

  rs.setAlphaTestRef( 0.05 );

  bool trasparentZW = false;
#ifdef __ANDROID__
  //rs.setBlend(1);
  trasparentZW = true;
#endif

  if( m==Tempest::addBlend ){
    rs.setBlend(1);
    rs.setZWriting(0);
    rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::one,
                     Tempest::RenderState::AlphaBlendMode::one );
    }

  if( m==Tempest::alphaBlend ){
    rs.setBlend(1);
    rs.setZWriting(trasparentZW);
    }

  if( m==Tempest::multiplyBlend ){
    rs.setBlend(1);
    rs.setZWriting(trasparentZW);

    rs.setBlendMode( Tempest::RenderState::AlphaBlendMode::src_color,
                     Tempest::RenderState::AlphaBlendMode::dst_color );
    }

  return rs;
  }
