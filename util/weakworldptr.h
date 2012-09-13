#ifndef WEAKWORLDPTR_H
#define WEAKWORLDPTR_H

#include <memory>

class World;
class GameObject;

class WeakWorldPtr {
  public:
    WeakWorldPtr();
    WeakWorldPtr( World& w, std::shared_ptr<GameObject>& obj  );
    WeakWorldPtr( const WeakWorldPtr& other );
    ~WeakWorldPtr();

    WeakWorldPtr& operator = ( const WeakWorldPtr& other );

    GameObject& value();
    operator bool () const;
  private:
    std::shared_ptr<GameObject> v;
    World * w;

  friend class World;
  };

#endif // WEAKWORLDPTR_H
