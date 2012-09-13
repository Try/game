#include "weakworldptr.h"

#include "game/world.h"

WeakWorldPtr::WeakWorldPtr() {
  w = 0;
  }

WeakWorldPtr:: WeakWorldPtr(World &wd, std::shared_ptr<GameObject> &obj)
             : v(obj), w(&wd) {
  w->addWptr( this );
  }

WeakWorldPtr::WeakWorldPtr(const WeakWorldPtr &other):v( other.v), w(other.w) {
  if( w )
    w->addWptr( this );
  }

WeakWorldPtr::~WeakWorldPtr() {
  if( w )
    w->delWptr( this );
  }

WeakWorldPtr &WeakWorldPtr::operator = (const WeakWorldPtr &other) {
  if( this==&other )
    return *this;

  if( w )
    w->delWptr( this );

  w = other.w;
  v = other.v;

  if( w )
    w->addWptr( this );
  return *this;
  }

GameObject &WeakWorldPtr::value() {
  return *v;
  }

WeakWorldPtr::operator bool() const {
  return v;
  }
