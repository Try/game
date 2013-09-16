#include "materialserver.h"

#include "algo/algo.h"
#include "resource.h"
#include "translate/mxassembly.h"

#include "gbufffer.h"
#include "frustum.h"

MaterialServer::ObjList::ObjList() {
  m_size = 0;
  obj.reserve(256);
  }

void MaterialServer::ObjList::insert(const AbstractGraphicObject *b) {
  size_t id = b->vboHandle();
  if( !id )
    return;

  ++m_size;

  for( size_t i=0; i<obj.size(); ++i ){
    if( obj[i].vboId==id ){
      obj[i].obj.push_back(b);
      return;
      }
    }

  obj.resize( obj.size()+1 );
  obj.back().obj.reserve(512);
  obj.back().vboId = id;
  obj.back().obj.push_back(b);
  }

size_t MaterialServer::ObjList::size() const {
  return m_size;
  }

void MaterialServer::ObjList::clear() {
  for( size_t i=0; i<obj.size(); ++i )
    obj[i].obj.clear();

  m_size = 0;
  }

void MaterialServer::ObjList::collect() {
  size_t count = 0;
  for( size_t i=0; i<obj.size(); ++i )
    if( obj[i].obj.size() ){
      obj[count].vboId = obj[i].vboId;
      ++count;
      }

  obj.resize( count );
  }

MaterialServer::MaterialServer( Tempest::VertexShaderHolder &vsHolder,
                                Tempest::FragmentShaderHolder &fsHolder,
                                Tempest::LocalTexturesHolder  & localTex,
                                ShaderSource::Lang lang )
  : vsHolder(vsHolder),
    fsHolder(fsHolder),
    localTex(localTex),
    lang(lang),
    res(0) {
  }

size_t MaterialServer::idOfMaterial( const std::string &m ) {
  return matId[m];
  }

void MaterialServer::load( Resource &r ) {
  res = &r;
  mat.clear();
  efect.clear();

  gbufPass.clear();
  sspass.clear();
  blpass.clear();

  auto m = r.unitsMaterials();
  for( auto i = m.begin(); i!=m.end(); ++i )
    pushMaterial(*res, *i);

  blt        = pushEfect(*res, "blt");
  blt_to_tex = pushEfect(*res, "blt_to_tex");
  mirron_blt = pushEfect(*res, "mirron_blt");

  shadow_main[1] = idOfMaterial("shadow_main");
  shadow_main[2] = idOfMaterial("shadow_bias");

  {
    Tempest::RenderState r = mat[shadow_main[2]]->m.renderState();
    r.setCullFaceMode( Tempest::RenderState::CullMode::front );
    mat[shadow_main[2]]->m.setRenderState(r);
  }

  main_material  = idOfMaterial("main");
  water          = idOfMaterial("water");

  hWater[0] = r.texture("water/height0");
  hWater[1] = r.texture("water/height1");

  Mat *waterM = mat[water].get();
  sswater = find( sspass, waterM )!=sspass.end();

  remove( sspass,   waterM );
  remove( gbufPass, waterM );
  }

void MaterialServer::draw( const AbstractGraphicObject &obj,
                           bool spass ) {
  if( spass ){
    if( obj.material().shMatID )
      mat[ shadow_main[obj.material().shMatID] ]->obj.insert(&obj);
    } else {
    if( obj.material().matID != size_t(-1) )
      mat[obj.material().matID]->obj.insert(&obj); else
      mat[main_material       ]->obj.insert(&obj);
    }
  }

template< class M, class ... Args >
void MaterialServer::drawTo( M & m,
                             Tempest::Device & device,
                             bool clr,
                             ShaderMaterial::UniformsContext& context,
                             Args&...  args ){
  //Mat &m = *gbufPass[i];
  Tempest::Render render( device, args..., m.m.vs, m.m.fs );
  render.setRenderState( m.m.renderState() );
  if(clr)
    render.clear(Tempest::Color(0), 1);
    //device.clearZ(1);
  draw( m, m, render, context );
  }

template< class ... Args >
void MaterialServer::completeDrawMMat(Tempest::Device &device,
                                       const Scene &/*scene*/,
                                       ShaderMaterial::UniformsContext &context,
                                       ShaderMaterial::UniformsContext &/*econtext*/,
                                       Args&... g ){
  Mat *waterM = mat[water].get();

  for( size_t i=0; i<gbufPass.size(); ++i ){
    Mat &m = *gbufPass[i];
    drawTo(m, device, i==0, context, g... );
    }

  for( size_t i=0; i<blpass.size(); ++i ){
    Mat &m = *blpass[i];
    drawTo(m, device, i==0 && gbufPass.size()==0, context, g... );
    }

  if( !sswater ){
    context.texture[1][0] = hWater[0];
    context.texture[1][1] = hWater[1];
    drawTo( *waterM, device, false, context );
    }
  }

void MaterialServer::completeDraw( Tempest::Device & device,
                                   const Scene &scene,
                                   GBuffer &g,
                                   Tempest::Texture2d* screen,
                                   Tempest::Texture2d* depth,
                                   ShaderMaterial::UniformsContext& context,
                                   ShaderMaterial::UniformsContext& econtext ) {
  Mat *waterM = mat[water].get();
  Tempest::Texture2d buf, *rTaget = screen;

  if( screen && settings.api==GraphicsSettingsWidget::Settings::directX ){
    buf = localTex.create( screen->size(), screen->format() );
    rTaget = &buf;
    }

  size_t countSS = 0;
  for( size_t i=0; i<sspass.size(); ++i )
    countSS += sspass[i]->obj.size();
  if( sswater )
    countSS += waterM->obj.size();

  if( countSS ){
    completeDrawMMat( device, scene, context, econtext, g.color[0], g.depth );
    execEfect(blt_to_tex, device, rTaget, econtext, g.color[0]);
    } else {
    if( screen )
      completeDrawMMat( device, scene, context, econtext, *rTaget, *depth ); else
      completeDrawMMat( device, scene, context, econtext );
    }

  if( countSS ){
    Tempest::Texture2d r = localTex.create( g.color[0].size(),
                                            g.color[0].format() );
    execEfect(blt, device, &r, econtext, g.color[0]);
    context.texture[ ShaderSource::tsScreenData ][0] = g.color[0];

    for( size_t i=0; i<sspass.size(); ++i ){
      Mat &m = *sspass[i];
      drawTo(m, device, false, context, r, g.depth );
      }

    if( sswater ){
      context.texture[1][0] = hWater[0];
      context.texture[1][1] = hWater[1];
      drawTo( *waterM, device, false, context, r, g.depth );
      }

    if( screen )
      execEfect(blt_to_tex, device, rTaget, econtext, r); else
      execEfect(blt, device, screen, econtext, r);
    }

  if( screen && rTaget!=screen ){
    execEfect(mirron_blt, device, screen, econtext, *rTaget);
    }

  for( size_t i=0; i<mat.size(); ++i )
    mat[i]->obj.clear();
  }

void MaterialServer::makeWaterHMap( Tempest::Device &device,
                                    const Scene &,
                                    ShadowBuffer &g,
                                    ShaderMaterial::UniformsContext &context,
                                    ShaderMaterial::UniformsContext & ){
  {
    Mat &m = *mat[ water ];
    {
    Tempest::Render render(device, g.z, g.depth, m.m.vs, m.m.fs);
    render.clear( Tempest::Color(1,1,1,0), 1 );
    }

    for( size_t i=0; i<gbufPass.size(); ++i ){
      Mat &m1 = *gbufPass[i];
      drawTo( m1, device, false, context, g.z, g.depth );
      //draw( m1, m1, render, context );
      }

    for( size_t i=0; i<mat.size(); ++i )
      mat[i]->obj.clear();
  }

  //execEfect(device, econtext, g.z);
  }

void MaterialServer::completeDraw( Tempest::Device &device,
                                   const Scene &,
                                   ShadowBuffer &g,
                                   ShaderMaterial::UniformsContext &context,
                                   ShaderMaterial::UniformsContext & ){
  {
    Mat &m = *mat[ shadow_main[2] ];
    {
    Tempest::Render render(device, g.z, g.depth, m.m.vs, m.m.fs);
    render.clear( Tempest::Color(1,1,1,0), 1 );
    }

    for( size_t i=0; i<gbufPass.size(); ++i ){
      Mat &m1 = *gbufPass[i];
      drawTo( m1, device, false, context, g.z, g.depth );
      //draw( m1, m1, render, context );
      }

    for( size_t i=0; i<mat.size(); ++i )
      mat[i]->obj.clear();
  }

  //execEfect(device, econtext, g.z);
  }

void MaterialServer::execEfect( size_t efectId,
                                Tempest::Device &dev,
                                Tempest::Texture2d* out,
                                ShaderMaterial::UniformsContext &context,
                                const Tempest::Texture2d& t ) {
  PostProc &p = *efect[efectId];

  context.texture[ShaderSource::tsDiffuse][0] = t;

  p.m.setupShaderConst(context);

  if( out )
    dev.beginPaint(*out); else
    dev.beginPaint();

  dev.setRenderState( Tempest::RenderState::PostProcess );
  dev.drawFullScreenQuad( p.m.vs, p.m.fs );
  dev.endPaint();
  }

void MaterialServer::draw( MaterialServer::Mat &m,
                           MaterialServer::Mat &obj,
                           Tempest::Render &render,
                           ShaderMaterial::UniformsContext &context ) {
  for( size_t i=0; i<obj.obj.obj.size(); ++i ){
    size_t rmax = obj.obj.obj[i].obj.size();
    std::vector<const AbstractGraphicObject*>& x = obj.obj.obj[i].obj;
    for( size_t r=0; r<rmax; ++r ){
      const AbstractGraphicObject& ptr = *x[r];
      setupObjectConstants( ptr, context);
      m.m.setupShaderConst(context);
      render.draw( ptr );
      }
    }
  }

void MaterialServer::setupObjectConstants( const AbstractGraphicObject &obj,
                                           ShaderMaterial::UniformsContext& context ){
  Tempest::Matrix4x4& mWorld = context.mWorld,
                    & object = context.object;

  object = obj.transform();

  Tempest::Matrix4x4& mvp = context.mvp;
  mvp = context.proj;
  mvp.mul( mWorld );
  mvp.mul( object );

  context.shMatrix = context.shView;
  context.shMatrix.mul( object );

  context.texture[0][0] = obj.material().diffuse;
  context.texture[1][0] = obj.material().normal;

  context.texture[ShaderSource::tsEmission][0] = obj.material().emission;

  context.texID = 0;
  }

void MaterialServer::pushMaterial( Resource &res, const std::string &s ) {
  std::shared_ptr<Mat> m = std::make_shared<Mat>(vsHolder, fsHolder);
  MxAssembly assemb( MVertex::decl(), lang );

  m->code = res.material(s);
  CompileOptions opt;
  opt.lang = lang;
  setupOpt(opt);
  m->m.install( *m->code.codeOf(), opt, assemb );

  T_ASSERT( m->m.vs.isValid() && m->m.fs.isValid() );

  matId[s] = mat.size();
  mat.push_back( m );

  bool ssNeed = false;
  for( size_t i=0; i<m->m.texUniformCount(); ++i )
    if( m->m.texUniform(i).texSemantic==ShaderSource::tsScreenData )
      ssNeed = true;

  if( !ssNeed ){
    if( m->m.renderState()==Tempest::RenderState() )
      gbufPass.push_back(m.get()); else
      blpass.push_back(m.get());
    } else {
      sspass.push_back(m.get());
    }
  }

size_t MaterialServer::pushEfect(Resource &res, const std::string &s) {
  std::shared_ptr<PostProc> m = std::make_shared<PostProc>(vsHolder, fsHolder);
  VertexInputAssembly assemb( Tempest::DefaultVertex::decl(), lang );

  m->code = res.material(s);
  CompileOptions opt;
  opt.lang = lang;
  setupOpt(opt);
  m->m.install( *m->code.codeOf(), opt, assemb );

  efect.push_back( m );

  return efect.size()-1;
  }

void MaterialServer::setupOpt(CompileOptions &opt) {
  if( GraphicsSettingsWidget::Settings::api==GraphicsSettingsWidget::Settings::directX )
    opt.options.insert("directX"); else
    opt.options.insert("openGL");

  opt.options.insert("oesRender");
#ifdef __ANDROID__
  opt.options.insert("oesRender");
#endif
  if( settings.normalMap ){
    opt.options.insert("bump");
    opt.options.insert("unitBump");
    }

  if( settings.shadowMapRes>0 )
    opt.options.insert("shadow");
  }

void MaterialServer::setSettings( const GraphicsSettingsWidget::Settings &s ) {
  settings = s;
  load(*res);
  }

void MaterialServer::finishFrame() {

  }
