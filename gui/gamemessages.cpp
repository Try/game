#include "gamemessages.h"

#include "font.h"
#include "lang/lang.h"
#include <Tempest/Painter>
#include "game.h"

std::vector<GameMessages::MSG> GameMessages::msg;
std::unordered_set<GameMessages*> GameMessages::views;

unsigned int GameMessages::ticksCount = 0;

GameMessages::GameMessages( Resource & res ):res(res) {
  msg.reserve(64);
  msg.clear();
  cride.data = res.pixmap("gui/cride");

  //message(L"msg");
  views.insert(this);
  }

GameMessages::~GameMessages() {
  views.erase(this);
  }

void GameMessages::message(const std::wstring &txt) {
  MSG m;
  m.str = Lang::tr(txt);
  m.tPrint = ticksCount;

  msg.push_back(m);
  updateAll();
  }

void GameMessages::message(const std::wstring &txt, PixmapsPool::TexturePtr icon) {
  MSG m;
  m.str    = Lang::tr(txt);
  m.icon.data   = icon;
  m.tPrint = ticksCount;

  msg.push_back(m);
  updateAll();
  }

void GameMessages::tickAll() {
  ++ticksCount;

  for( size_t i=0; i<msg.size(); ++i ){
    if( unsigned(ticksCount - msg[i].tPrint) > unsigned(Game::ticksPerSecond*4) ){
      msg.erase( msg.begin()+i );
      updateAll();
      }
    }
  }

void GameMessages::paintEvent(Tempest::PaintEvent &e) {
  Tempest::Painter p(e);

  if( owner() ){
    Tempest::Rect r = Tempest::Rect( -x(), -y(), owner()->w(), owner()->h() );
    p.setScissor(r);
    }

  Font f;

  int h1 = h()-30;
  p.setFont( Font() );
  p.setBlendMode( Tempest::alphaBlend );

  for( size_t id=0; id<msg.size(); ++id ){
    size_t i = msg.size()-id-1;

    int iconSz = 25;
    int dh = std::max(iconSz, f.textSize(res, msg[i].str).h);

    if( msg[i].icon.data.tex ){
      p.setTexture( cride );
      p.drawRect(0, h1-(dh+iconSz)/2, iconSz, iconSz,
                 0,0, cride.data.rect.w, cride.data.rect.h );

      p.setTexture( msg[i].icon );
      float k1 = msg[i].icon.data.rect.w/float(iconSz),
            k2 = msg[i].icon.data.rect.h/float(iconSz);

      float k = 1.0/std::max(k1, k2);
      int w = msg[i].icon.data.rect.w*k,
          h = msg[i].icon.data.rect.h*k;

      p.drawRect( (iconSz-w)/2, h1-(dh+h)/2,
                  w,h,
                  0, 0, msg[i].icon.data.rect.w, msg[i].icon.data.rect.h );
      p.drawText( iconSz, h1-dh, msg[i].str );
      } else {
      p.drawText( 0, h1-dh, msg[i].str );
      }

    h1-=dh;
    }
  //p.drawText(0,0, "abs");
  }

void GameMessages::tick() {

  }

void GameMessages::updateAll() {
  for( auto i=views.begin(); i!=views.end(); ++i )
    (*i)->update();
  }