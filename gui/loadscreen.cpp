#include "loadscreen.h"

#include "resource.h"
#include <Tempest/Painter>
#include <Tempest/LocalTexturesHolder>
#include <Tempest/Application>

LoadScreen::LoadScreen(Resource &res,
                       Tempest::LocalTexturesHolder& tex ):res(res) {
  bg   = tex.load("data/textures/gui/paper.png");
  t    = Tempest::Application::tickCount();
  anim = 0;
  }

void LoadScreen::paintEvent(Tempest::PaintEvent &e) {
  Tempest::Painter p(e);

  //p.drawLine(0,0, 1000,1000);
  p.setTexture(bg);
  p.drawRect(0,0, w(), h());
  p.setFont( Tempest::Font() );

  const char* s[] = {
    "Loading",
    "Loading.",
    "Loading..",
    "Loading..."
    };
  p.drawText(100,100, s[anim] );
  if( Tempest::Application::tickCount()-t>500 ){
    t  = Tempest::Application::tickCount();
    anim = (anim+1)%4;
    }

  //p.unsetTexture();
  //p.drawLine(0,0, 1000,1000);
  }
