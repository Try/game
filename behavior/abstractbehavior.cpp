#include "abstractbehavior.h"

AbstractBehavior::~AbstractBehavior()
{
}

void AbstractBehavior::atackMoveEvent(MoveEvent &m) {
  m.ignore();
  }

void AbstractBehavior::atackMoveEvent(MoveSingleEvent &m) {
  m.ignore();
  }

void AbstractBehavior::atackContinueEvent(MoveSingleEvent &m) {
  m.ignore();
  }

void AbstractBehavior::moveEvent(MoveEvent &m) {
  m.ignore();
  }

void AbstractBehavior::moveEvent(MoveToUnitEvent &m) {
  m.ignore();
  }

void AbstractBehavior::moveEvent( PatrulEvent &m) {
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

void AbstractBehavior::holdEvent(HoldEvent &m) {
  m.ignore();
  }

void AbstractBehavior::cancelEvent(CancelEvent &m) {
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

  if( m==AtackMove ){
    MoveSingleEvent mm;
    mm.modif = md;
    mm.x = x;
    mm.y = y;

    atackMoveEvent(mm);
    return mm.isAccepted();
    }

  if( m==AtackMoveContinue ){
    MoveSingleEvent mm;
    mm.modif = md;
    mm.x = x;
    mm.y = y;

    atackContinueEvent(mm);
    return mm.isAccepted();
    }

  if( m==AtackMoveGroup ){
    MoveEvent mm;
    mm.modif = md;
    mm.x = x;
    mm.y = y;

    atackMoveEvent(mm);
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

  if( m==Patrul ){
    PatrulEvent mm;
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

  if( m==Hold ){
    HoldEvent mm;
    holdEvent(mm);
    return mm.isAccepted();
    }

  if( m==Cancel ){
    CancelEvent mm;
    cancelEvent(mm);
    return mm.isAccepted();
    }

  return 0;
  }

bool AbstractBehavior::message( Message m,
                                size_t id,
                                AbstractBehavior::Modifers md) {
  if( m==ToUnit ){
    MoveToUnitEvent mm;
    mm.modif = md;
    mm.id = id;

    moveEvent(mm);
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
