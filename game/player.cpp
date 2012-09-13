#include "player.h"

#include "algo/algo.h"

Player::Player(int num) {
  editObj  = 0;

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
  m.color = MyGL::Color( 255*m.color.r(),
                         255*m.color.g(),
                         255*m.color.b() );
  }

void Player::addUnit(GameObject *u) {
  m.objects.push_back(u);
  }

void Player::delUnit(GameObject *u) {
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

  onUnitSelected( selected() );
  }

std::vector<GameObject*> &Player::selected() {
  return m.selected;
  }

const MyGL::Color &Player::color() const {
  return m.color;
  }
