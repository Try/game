#include "abstractbehavior.h"

AbstractBehavior::~AbstractBehavior()
{
}

void AbstractBehavior::moveEvent(MoveEvent &m) {
  m.ignore();
  }

void AbstractBehavior::moveEvent(MoveSingleEvent &m) {
  m.ignore();
  }

void AbstractBehavior::moveEvent(MineralMoveEvent &m) {
  m.ignore();
  }

void AbstractBehavior::stopEvent(StopEvent &m) {
  m.ignore();
  }

void AbstractBehavior::repositionEvent(RepositionEvent &m) {
  m.ignore();
  }

void AbstractBehavior::positionChangeEvent(PositionChangeEvent &m) {
  m.ignore();
  }

bool AbstractBehavior::message( AbstractBehavior::Message m,
                                int x, int y,
                                AbstractBehavior::Modifers md ) {
  if( m==Move ){
    MoveEvent mm;
    mm.modif = md;
    mm.x = x;
    mm.y = y;

    moveEvent(mm);
    return mm.isAccepted();
    }

  if( m==MoveSingle ){
    MoveSingleEvent mm;
    mm.modif = md;
    mm.x = x;
    mm.y = y;

    moveEvent(mm);
    return mm.isAccepted();
    }

  if( m==MineralMove ){
    MineralMoveEvent mm;
    mm.modif = md;
    mm.x = x;
    mm.y = y;

    moveEvent(mm);
    return mm.isAccepted();
    }

  if( m==Reposition ){
    RepositionEvent mm;
    mm.modif = md;
    mm.x = x;
    mm.y = y;

    repositionEvent(mm);
    return mm.isAccepted();
    }

  if( m==onPositionChange ){
    PositionChangeEvent mm;
    mm.modif = md;
    mm.x = x;
    mm.y = y;

    positionChangeEvent(mm);
    return mm.isAccepted();
    }

  if( m==StopMove ){
    StopEvent mm;
    stopEvent(mm);
    return mm.isAccepted();
    }

  return 0;
  }

bool AbstractBehavior::message( AbstractBehavior::Message ,
                                const std::string &,
                                AbstractBehavior::Modifers ) {
  return 0;
  }


BehaviorEvent::BehaviorEvent() {
  accept();
  }

void BehaviorEvent::accept() {
  acpt = true;
  }

void BehaviorEvent::ignore() {
  acpt = false;
  }

bool BehaviorEvent::isAccepted() const {
  return acpt;
  }
