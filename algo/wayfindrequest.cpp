#include "wayfindrequest.h"

#include "game/gameobject.h"
#include "landscape/terrain.h"

#include "algo.h"
#include "wayfindalgo.h"
#include "behavior/movebehavior.h"

WayFindRequest::WayFindRequest() {
  visited.reserve(400);
  group  .reserve(400);
  }

void WayFindRequest::findWay( int x, int y, GameObject *obj ) {
  Tg tg;
  tg.x = x/Terrain::quadSize;
  tg.y = y/Terrain::quadSize;

  rq[tg].push_back(obj);
  }

void WayFindRequest::tick( const Terrain &t ) {
  for( auto i=rq.begin(); i!=rq.end(); ++i ){
    findWay( i->first.x, i->first.y, i->second, t );
    }

  rq.clear();
  }

void WayFindRequest::findWay( int x, int y,
                              std::vector<GameObject *> &objs,
                              const Terrain &t ) {
  visited.clear();
  visited.resize(objs.size(), 0);

  for( size_t i=0; i<objs.size(); ++i ){
    if( !visited[i] ){
      visited[i] = true;
      makeGroup( objs[i], objs, visited, group );
      findWayGr( x, y, group, t );
      }
    }
  }

void WayFindRequest::makeGroup( GameObject * obj,
                                std::vector<GameObject *> &objs,
                                std::vector<bool> & visited,
                                std::vector<GameObject *> &gr ) {
  gr.clear();
  gr.push_back(obj);
  //return;

  std::vector< GameObject* > stk[2], *stk1, *stk2;
  stk[0].push_back(obj);

  stk1 = &stk[0];
  stk2 = &stk[1];

  while( stk1->size() ){
    for( size_t i=0; i<stk1->size(); ++i ){
      GameObject & ob = *stk1->at(i);
      for( size_t r=0; r<ob.colisions.size(); ++r )
        for( size_t q=0; q<objs.size(); ++q ){
          if( objs[q]==ob.colisions[r] && !visited[q] ){
            gr.push_back( objs[q] );
            visited[q] = true;
            stk2->push_back( objs[q] );
            }
          }
      }

    std::swap(stk1, stk2);
    stk2->clear();
    }
  }

void WayFindRequest::findWayGr( int x, int y,
                                std::vector<GameObject *> &objs,
                                const Terrain &t) {
  int qx = x*Terrain::quadSize,
      qy = y*Terrain::quadSize;

  int d = objs[0]->distanceSQ(qx,qy);
  GameObject *obj = objs[0];

  for( size_t i=0; i<objs.size(); ++i ){
    int d2 = objs[i]->distanceSQ(qx,qy);

    if( d2<d ){
      d = d2;
      obj = objs[i];
      }
    }

  WayFindAlgo algo(t);
  algo.findWay( *obj,
                obj->x()/Terrain::quadSize,
                obj->y()/Terrain::quadSize,
                x,
                y );

  for( size_t i=0; i<objs.size(); ++i ){
    MoveBehavior *b = objs[i]->behavior.find<MoveBehavior>();
    if( b )
      b->setWay( algo.way );
    }
  }
