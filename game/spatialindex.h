#ifndef SPATIALINDEX_H
#define SPATIALINDEX_H

#include <vector>
#include <memory>

class GameObject;

class SpatialIndex {
  public:
    typedef std::shared_ptr<GameObject> PGameObject;
    SpatialIndex( int w, int h );

    void fill( std::vector<PGameObject> & obj );
    void clear();

    void solveColisions();
  private:
    std::vector<size_t>      psum;
    std::vector<GameObject*> obj;

    int w,h, sizeMax;

    void solveColisions(GameObject* , size_t id);
    bool hasEffect( GameObject* tg, GameObject* obj );
  };

#endif // SPATIALINDEX_H
