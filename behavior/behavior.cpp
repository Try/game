#include "behavior.h"

#include "game/gameobject.h"
#include "util/math.h"
#include "util/factory.h"

#include "util/behaviorsfactory.h"
#include "behavior/movebehavior.h"
#include "algo/algo.h"

Behavior::Behavior():findex( BehaviorsFactory::productsCount() ){
  object = 0;
  clos   = 0;
  }

Behavior::~Behavior() {
  clear();
  }

void Behavior::bind( GameObject &gameObj,
                     Closure & c ) {
  object = &gameObj;
  clos   = &c;

  c.isOnMove    = false;
  c.isReposMove = false;
  c.isMoviable  = false;
  c.isMineralMove = false;
  }

void Behavior::add(const std::string &n) {
  size_t id;
  AbstractBehavior* b = BehaviorsFactory::create( n, id, *object, *clos );
  if( b ){
    behaviors.push_back( b );
    findex[id] = b;
    }
  }

void Behavior::del(AbstractBehavior *ptr) {
  AbstractBehavior* b = 0;

  size_t r=0;
  for( size_t i=0; i<behaviors.size(); ++i ){
    behaviors[r] = behaviors[i];

    if( behaviors[i]==ptr )
      b = behaviors[i]; else
      ++r;
    }

  for( size_t i=0; i<findex.size(); ++i ){
    if( findex[i]==b )
      findex[i] = 0;
    }

  delete b;
  behaviors.resize(r);
  }

void Behavior::clear() {
  for( size_t i=0; i<behaviors.size(); ++i )
    delete behaviors[i];
  behaviors.clear();
  std::fill( findex.begin(), findex.end(), (AbstractBehavior*)0 );
  }

bool Behavior::message( Message msg, int x, int y, Modifers md ) {
  for( size_t i=behaviors.size(); i>0; --i )
    if( behaviors[i-1]->message( msg, x, y, md ) )
      return 1;

  return 0;
  }

bool Behavior::message( Message msg, size_t id,
                        AbstractBehavior::Modifers md) {
  for( size_t i=behaviors.size(); i>0; --i )
    if( behaviors[i-1]->message( msg, id, md ) )
      return 1;

  return 0;
  }

bool Behavior::message( AbstractBehavior::Message msg,
                        const std::string &s,
                        AbstractBehavior::Modifers md) {
  for( size_t i=behaviors.size(); i>0; --i )
    if( behaviors[i-1]->message( msg, s, md ) )
      return 1;

  return 0;
  }

void Behavior::tick(const Terrain &terrain) {
  for( size_t i=behaviors.size(); i>0; --i )
    behaviors[i-1]->tick( terrain );
  }
