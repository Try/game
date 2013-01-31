#include "player.h"

#include "algo/algo.h"
#include "gameobject.h"

#include "protoobject.h"

#include "util/gameserializer.h"
#include "world.h"

Player::Player(int num) {
  editObj  = 0;

  m.gold   = 100500;
  //m.gold   = 50;
  m.lim    = 0;
  m.limMax = 0;

  m.team   = num;
  m.num    = num;
  m.htCtrl = 0;

  static MyGL::Color cl[8] = {
    MyGL::Color(1, 1, 1 ),
    MyGL::Color(1, 1, 0 ),
    MyGL::Color(1, 0, 0 ),
    MyGL::Color(0, 1, 0 ),
    MyGL::Color(0, 0, 1 ),
    MyGL::Color(1, 0, 1 ),
    MyGL::Color(0, 1, 1 ),
    MyGL::Color(0, 0, 0 ),
    };

  m.color = cl[ std::min(7, m.num) ];
  /*
  m.color = MyGL::Color( 255*m.color.r(),
                         255*m.color.g(),
                         255*m.color.b() );*/
  //setSyncFlag(0);
  }

void Player::addUnit(GameObject *u) {
  m.objects.push_back(u);
  m.lim -= u->getClass().data.lim;
  m.lim += u->getClass().data.limInc;

  m.limMax += u->getClass().data.limInc;
  }

void Player::delUnit(GameObject *u) {
  onUnitDied(*u, *this);

  m.lim += u->getClass().data.lim;
  m.lim -= u->getClass().data.limInc;
  m.limMax -= u->getClass().data.limInc;

  remove( m.objects,  u );
  remove( m.selected, u );
  }

int Player::team() const {
  return m.team;
  }

void Player::setTeam(int t) {
  m.team = t;
  }

bool Player::hasHostControl() const {
  return m.htCtrl;
  }

void Player::setHostCtrl(bool c) {
  m.htCtrl = c;
  }

void Player::select(GameObject *obj, bool s ) {
  if( s )
    m.selected.push_back(obj); else
    remove( m.selected, obj);

  std::sort( m.selected.begin(),
             m.selected.end(),
             Player::compare );

  onUnitSelected( selected(), *this );
  }

std::vector<GameObject*> &Player::selected() {
  return m.selected;
  }

const MyGL::Color &Player::color() const {
  return m.color;
  }

void Player::addGold(int g) {
  m.gold += g;
  }

int Player::gold() const {
  return m.gold;
  }

int Player::lim() const {
  return m.lim;
  }

void Player::addLim(int l) {
  m.lim += l;
  }

void Player::incLim(int l) {
  m.lim += l;
  m.limMax += l;
  }

bool Player::canBuild( const ProtoObject &p ) const {
  return (  m.gold>= p.data.gold &&
           (m.lim >= p.data.lim || p.data.lim<=0));
  }

int Player::limMax() const {
  return m.limMax;
  }

void Player::serialize(GameSerializer &s) {
  s + m.gold + m.team + m.htCtrl + m.num;  
  //m.gold = 50;
  }

size_t Player::unitsCount() const {
  return m.objects.size();
  }

GameObject &Player::unit(size_t id) {
  return *m.objects[id];
  }

int Player::number() const {
  return m.num;
  }

const MyGL::Pixmap &Player::fog() const {
  return m.fog;
  }

bool Player::compare(const GameObject *a, const GameObject *b) {
  return a->getClass().name < b->getClass().name;
  }

void Player::tick( World &curW ) {

  }

void Player::computeFog(void *curW) {
  fillFog(m.fog, *(World*)curW);
  }

void Player::fillFog(MyGL::Pixmap &p, World &wx ) {
  const bool useFog = 1;

  if( p.width()  != wx.terrain().width() ||
      p.height() != wx.terrain().height() ){
    p = MyGL::Pixmap( wx.terrain().width(),
                      wx.terrain().height(),
                      true );

    int cf = useFog? 0:255;
    MyGL::Pixmap::Pixel pix;
    pix.r = cf;
    pix.g = cf;
    pix.b = cf;
    pix.a = cf;

    for( int i=0; i<p.width(); ++i )
      for( int r=0; r<p.height(); ++r ){
        p.set(i,r, pix);
        }

    }

  //return;

  int cf2 = useFog ? 128:255;//128;
  MyGL::Pixmap::Pixel pix;
  pix.r = cf2;
  pix.g = cf2;
  pix.b = cf2;
  pix.a = cf2;

  for( int i=0; i<p.width(); ++i )
    for( int r=0; r<p.height(); ++r ){
      if( p.at(i,r).a>cf2 )
        p.set(i,r, pix);
      }

  const std::vector<World::PGameObject> & objects = wx.activeObjects();
  int qs = Terrain::quadSize;

  for( size_t i=0; i<objects.size(); ++i ){
    const GameObject& obj = *objects[i];

    int x = obj.x()/qs,
        y = obj.y()/qs;

    if( obj.team()==team() ){
      cride( p, x, y, obj.getClass().data.visionRange );
      }
    }
  }

void Player::cride(MyGL::Pixmap &p,
                    int x, int y, int R ) {
  MyGL::Pixmap::Pixel pix;
  pix.r = 255;
  pix.g = 255;
  pix.b = 255;
  pix.a = 255;

  MyGL::Pixmap::Pixel hpix[2] = {pix, pix};
  hpix[0].a = 128+64;
  hpix[0].r = hpix[0].a;
  hpix[0].g = hpix[0].a;
  hpix[0].b = hpix[0].a;

  hpix[1].a = 128-64;
  hpix[1].r = hpix[1].a;
  hpix[1].g = hpix[1].a;
  hpix[1].b = hpix[1].a;

  int lx = x - R,
      rx = x + R,
      ly = y - R,
      ry = y + R;

  int R1 = (R-2)*(R-2);
  int R2 = (R-1)*(R-1);
  int R3 = R*R;
  //R*=R;

  lx = std::max(2,lx);
  ly = std::max(2,ly);

  rx = std::min(p.width()-4,rx);
  ry = std::min(p.height()-4,ry);

  for( int i=lx; i<=rx; ++i )
    for( int r=ly; r<=ry; ++r ){
      int dx = i-x,
          dy = r-y;
      int dR = dx*dx+dy*dy;

      if( dR <= R1 )
        p.set(i,r, pix); else

      if( dR <= R2 && p.at(i,r).a<hpix[0].a )
        p.set(i,r, hpix[0]); else

      if( dR <= R3 && p.at(i,r).a<hpix[1].a )
        p.set(i,r, hpix[1]);
      }
  }
