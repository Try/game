#include "player.h"

#include "algo/algo.h"
#include "gameobject.h"

#include "protoobject.h"

#include "util/gameserializer.h"

Player::Player(int num) {
  editObj  = 0;

  m.gold = 100500;
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
  }

void Player::addUnit(GameObject *u) {
  m.objects.push_back(u);
  m.lim -= u->getClass().data.lim;
  m.lim += u->getClass().data.limInc;

  m.limMax += u->getClass().data.limInc;
  }

void Player::delUnit(GameObject *u) {
  onUnitDied(*u);

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

  onUnitSelected( selected() );
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
  s + m.gold + m.team + m.htCtrl + m.num
    ;

  }

bool Player::compare(const GameObject *a, const GameObject *b) {
  return a->getClass().name < b->getClass().name;
  }
